from matplotlib.pyplot import axis
import pandas as pd
import json

f = open('./project_campaign_new_1/project_campaign_new_1.json', 'r')
data = json.load(f)

df = pd.DataFrame(columns=['rtscts', 'standard', 'datarate',
                  'verbose', 'rng', 'thr1', 'pkl1', 'thr2', 'pkl2'])

for i in data['results']:
    id = data['results'][i]['meta']['id']
    file = open('./project_campaign_new_1/data/' + id + '/stdout', 'r')
    lines = file.readlines()
    thr1 = lines[6].split(':')[1].strip()
    pkl1 = lines[7].split(':')[1].strip()
    thr2 = lines[14].split(':')[1].strip()
    pkl2 = lines[15].split(':')[1].strip()
    df.loc[len(df)] = list(data['results'][i]['params'].values()) + \
        [thr1, pkl1, thr2, pkl2]

df.to_csv('stats_new_1.csv', sep='\t')
