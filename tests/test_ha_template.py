"""Evaluate the actual firmware template with HA-style state helpers."""
import json
import re
import unittest
from pathlib import Path
from jinja2 import Environment


class HomeAssistantTemplateTests(unittest.TestCase):
    def render(self, probe, values):
        source = (Path(__file__).parents[1] / 'arduino/BBQ-Control-V1/HomeAssistant.cpp').read_text()
        begin = source.index('String t=')
        end = source.index('cJSON *root=', begin)
        literals = re.findall(r'"(?:[^"\\]|\\.)*"', source[begin:end])
        template = json.loads(literals[0]) + probe + ''.join(map(json.loads, literals[1:]))
        env = Environment()
        env.filters['to_json'] = json.dumps
        return json.loads(env.from_string(template).render(
            states=lambda entity: values.get(entity, 'unavailable'),
            has_value=lambda entity: values.get(entity, 'unavailable') not in ('unknown', 'unavailable')))

    def test_all_probe_prefixes(self):
        for probe in ['meater_1', 'meater_2', 'meater_3', 'meater_4', 'meater_probe_0ae3b60f']:
            with self.subTest(probe=probe):
                values = {'sensor.' + probe + '_' + key: value for key, value in {
                    'innentemperatur': '54.5', 'umgebungstemperatur': '120',
                    'soll_temperatur': '65', 'kochstatus': 'started'}.items()}
                result = self.render(probe, values)
                self.assertTrue(result['probe_online'])
                self.assertEqual((result['core_c'], result['ambient_c'], result['target_c']), (54.5, 120, 65))
                self.assertEqual(result['cook_state'], 'started')
                self.assertIsNone(result['battery_pct'])

    def test_unavailable(self):
        result = self.render('meater_1', {})
        self.assertFalse(result['probe_online'])
        self.assertIsNone(result['core_c'])
        self.assertIsNone(result['cook_state'])

    def test_unknown_target_and_missing_ambient(self):
        result = self.render('meater_1', {'sensor.meater_1_innentemperatur': '0',
                                         'sensor.meater_1_soll_temperatur': 'unknown'})
        self.assertFalse(result['probe_online'])
        self.assertEqual(result['core_c'], 0)
        self.assertIsNone(result['target_c'])


if __name__ == '__main__':
    unittest.main()
