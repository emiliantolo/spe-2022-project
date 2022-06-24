import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import json
import math

f = open('./project_campaign_aaa/project_campaign_aaa.json', 'r')
data = json.load(f)

df = pd.DataFrame(columns=['id', 'thr1', 'thr2'])

for i in data['results']:
    id = data['results'][i]['meta']['id']
    file = open('./project_campaign_aaa/data/' + id + '/stdout', 'r')
    lines = file.readlines()
    thr1 = float(lines[45].split(':')[1].split('Mbps')[0].strip())
    thr2 = float(lines[53].split(':')[1].split('Mbps')[0].strip())

    thr3 = float(lines[81].split(':')[1].split('Mbps')[0].strip())
    thr4 = float(lines[89].split(':')[1].split('Mbps')[0].strip())

    df.loc[len(df)] = [id] + [thr1 + thr2, thr3 + thr4]

df_thr1 = df.thr1.agg(['mean', 'std']).reset_index()
df_thr2 = df.thr2.agg(['mean', 'std']).reset_index()

print(df_thr1)
print(df_thr2)
