"""Render the actual Blueprint payload with Home Assistant-like state fixtures.

This tests the template, not a running Home Assistant installation or MQTT broker.
Run: python -m unittest discover -s tests -p test_meater_blueprint.py
Dependencies: PyYAML 6.0.2 and Jinja2 3.1.6.
"""
from pathlib import Path
import datetime
import json
import unittest
import jinja2
import yaml


class BlueprintLoader(yaml.SafeLoader):
    pass


BlueprintLoader.add_constructor("!input", lambda loader, node: loader.construct_scalar(node))
BLUEPRINT = yaml.load((Path(__file__).resolve().parents[1] /
    "home-assistant/blueprints/automation/bbq_control/meater_to_mqtt.yaml").read_text(encoding="utf-8"),
    Loader=BlueprintLoader)
ENV = jinja2.Environment(undefined=jinja2.StrictUndefined)
ENV.filters["to_json"] = json.dumps
TEMPLATE = ENV.from_string(BLUEPRINT["actions"][0]["data"]["payload"])
NOW = datetime.datetime(2026, 9, 7, tzinfo=datetime.timezone.utc)


class State:
    def __init__(self, value, unit=None, age=0):
        self.state, self.unit = value, unit
        self.last_reported = NOW - datetime.timedelta(seconds=age)


class States(dict):
    def __call__(self, key):
        return self[key].state if key in self else "unknown"


def fixture():
    return {"sensor.core": State("54.2", "°C"), "sensor.ambient": State("120", "°C"),
            "sensor.target": State("65", "°C"), "sensor.cook": State("started"),
            "sensor.battery": State("80", "%")}


def render(values, **overrides):
    states = States(values)
    args = dict(core_entity="sensor.core", ambient_entity="sensor.ambient",
        target_entity="sensor.target", cook_entity="sensor.cook", battery_entity="sensor.battery",
        source_timeout=120, states=states,
        has_value=lambda key: key in states and states[key].state not in ["unknown", "unavailable"],
        state_attr=lambda key, attr: states[key].unit if key in states else None,
        as_timestamp=lambda value, default=0: value.timestamp() if value else default,
        now=lambda: NOW)
    args.update(overrides)
    return json.loads(TEMPLATE.render(**args))


class BlueprintTests(unittest.TestCase):
    def test_optional_selector_defaults(self):
        for key in ("target", "cook", "battery"):
            entry = BLUEPRINT["blueprint"]["input"][key + "_entity"]
            self.assertEqual(entry["default"], [])
            self.assertTrue(entry["selector"]["entity"]["multiple"])
            template = ENV.from_string(BLUEPRINT["variables"][key + "_entity"])
            self.assertEqual(template.render(**{key + "_selection": []}), "")
            self.assertEqual(template.render(**{key + "_selection": ["sensor.example"]}), "sensor.example")

    def test_celsius_snapshot(self):
        self.assertEqual(render(fixture()), dict(schema=1, probe_online=True, core_c=54.2,
            ambient_c=120, target_c=65, cook_state="started", battery_pct=80))

    def test_fahrenheit_conversion(self):
        values = fixture()
        values.update({"sensor.core": State("129.56", "°F"), "sensor.ambient": State("248", "°F"),
                       "sensor.target": State("149", "°F")})
        result = render(values)
        self.assertAlmostEqual(result["core_c"], 54.2)
        self.assertEqual((result["ambient_c"], result["target_c"]), (120, 65))

    def test_optional_inputs_not_selected(self):
        result = render(fixture(), target_entity="", cook_entity="", battery_entity="")
        self.assertTrue(result["probe_online"])
        self.assertIsNone(result["target_c"])
        self.assertIsNone(result["cook_state"])
        self.assertIsNone(result["battery_pct"])

    def test_unavailable_sensor(self):
        values = fixture()
        values["sensor.ambient"] = State("unavailable", "°C")
        result = render(values)
        self.assertFalse(result["probe_online"])
        self.assertIsNone(result["ambient_c"])

    def test_stale_and_future_reports(self):
        for age in (121, -10):
            values = fixture()
            values["sensor.core"] = State("54.2", "°C", age)
            self.assertFalse(render(values)["probe_online"])

    def test_missing_optional_entities(self):
        values = fixture()
        for key in ("sensor.target", "sensor.cook", "sensor.battery"):
            del values[key]
        result = render(values)
        self.assertIsNone(result["battery_pct"])
        self.assertIsNone(result["cook_state"])
        self.assertIsNone(result["target_c"])

    def test_delivery_contract(self):
        data = BLUEPRINT["actions"][0]["data"]
        self.assertFalse(data["retain"])
        self.assertEqual(data["qos"], 1)
        self.assertEqual(data["topic"], "bbq/control/v1/meater/state")
        self.assertEqual(BLUEPRINT["triggers"][1]["seconds"], "/30")


if __name__ == "__main__":
    unittest.main()
