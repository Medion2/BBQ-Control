#pragma once
// One bounded response, containing only the five supplied Meater devices.
static const char HaTemplate[]=R"HA(
{% set ns = namespace(rows=[]) %}
{% for name in ['meater_1','meater_2','meater_3','meater_4','meater_probe_0ae3b60f'] %}
{% set p = 'sensor.' ~ name ~ '_' %}
{% set c = states(p ~ 'innentemperatur') | float(none) %}
{% set a = states(p ~ 'umgebungstemperatur') | float(none) %}
{% set end = as_timestamp(states(p ~ 'verbleibende_zeit'), none) %}
{% set start = as_timestamp(states(p ~ 'verstrichene_zeit'), none) %}
{% set ns.rows = ns.rows + [{'schema':1, 'probe_online':c is number and a is number,
'core_c':c, 'ambient_c':a, 'target_c':states(p ~ 'soll_temperatur') | float(none),
'cook_state':states(p ~ 'kochstatus') if has_value(p ~ 'kochstatus') else none,
'battery_pct':none, 'peak_c':states(p ~ 'spitzentemperatur') | float(none),
'remaining_s':([0,end-as_timestamp(now())] | max) if end is number else none,
'elapsed_s':([0,as_timestamp(now())-start] | max) if start is number else none,
'cooking':states(p ~ 'kocht') if has_value(p ~ 'kocht') else none}] %}
{% endfor %}
{{ ns.rows | to_json }}
)HA";
