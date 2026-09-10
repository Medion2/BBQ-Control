import json
import unittest
from pathlib import Path
from datetime import datetime, timezone
from jinja2 import Environment


class V2TemplateTests(unittest.TestCase):
    def render(self, states):
        source=(Path(__file__).parents[1]/'arduino/BBQ-Control-V2/HaTemplate.h').read_text()
        template=source.split('R"HA(',1)[1].split(')HA"',1)[0]
        now=datetime(2026,9,10,12,tzinfo=timezone.utc)
        def stamp(value, default=None):
            if isinstance(value,datetime): return value.timestamp()
            try: return datetime.fromisoformat(value.replace('Z','+00:00')).timestamp()
            except (ValueError,AttributeError): return default
        env=Environment();env.filters['to_json']=json.dumps
        return json.loads(env.from_string(template).render(
            states=lambda e:states.get(e,'unavailable'),
            has_value=lambda e:states.get(e,'unavailable') not in ['unavailable','unknown'],
            now=lambda:now,as_timestamp=stamp))

    def test_unavailable_is_not_zero(self):
        rows=self.render({})
        self.assertEqual(len(rows),5)
        for row in rows:
            self.assertFalse(row['probe_online'])
            for name in ['core_c','ambient_c','target_c','remaining_s','elapsed_s','battery_pct']:
                self.assertIsNone(row[name])

    def test_probes_do_not_mix(self):
        values={}
        for i,name in enumerate(['meater_1','meater_2','meater_3','meater_4','meater_probe_0ae3b60f']):
            values['sensor.'+name+'_innentemperatur']=str(50+i)
            values['sensor.'+name+'_umgebungstemperatur']=str(100+i)
        rows=self.render(values)
        for i,row in enumerate(rows):
            self.assertTrue(row['probe_online'])
            self.assertEqual(row['core_c'],50+i)
            self.assertEqual(row['ambient_c'],100+i)

    def test_timestamps_and_extra_sensors(self):
        row=self.render({'sensor.meater_1_verbleibende_zeit':'2026-09-10T12:18:00+00:00',
                         'sensor.meater_1_verstrichene_zeit':'2026-09-10T11:00:00+00:00',
                         'sensor.meater_1_spitzentemperatur':'64.2',
                         'sensor.meater_1_kocht':'Steak',
                         'sensor.meater_1_kochstatus':'configured'})[0]
        self.assertEqual(row['remaining_s'],1080)
        self.assertEqual(row['elapsed_s'],3600)
        self.assertEqual(row['peak_c'],64.2)
        self.assertEqual(row['cooking'],'Steak')
        self.assertEqual(row['cook_state'],'configured')

    def test_past_end_and_invalid_start(self):
        row=self.render({'sensor.meater_1_verbleibende_zeit':'2026-09-10T11:00:00Z',
                         'sensor.meater_1_verstrichene_zeit':'unknown'})[0]
        self.assertEqual(row['remaining_s'],0)
        self.assertIsNone(row['elapsed_s'])


if __name__=='__main__': unittest.main()
