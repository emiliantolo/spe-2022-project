import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import json
import math

f = open('./project_campaign/project_campaign.json', 'r')
data = json.load(f)

df = pd.DataFrame(columns=['id', 'tcp', 'datarate', 'packetsize',
                  'verbose', 'rng', 'thr'])

for i in data['results']:
    id = data['results'][i]['meta']['id']
    file = open('./project_campaign/data/' + id + '/stdout', 'r')
    lines = file.readlines()
    thr1 = float(lines[6].split(':')[1].split('Mbps')[0].strip())
    thr2 = float(lines[14].split(':')[1].split('Mbps')[0].strip())
    df.loc[len(df)] = [id] + list(data['results'][i]
                                  ['params'].values()) + [thr1 + thr2]

df.to_csv('stats.csv', sep='\t')

packetsizes = np.sort(df['packetsize'].unique())
datarates = np.sort(df['datarate'].unique()) * 2

df_thr = df.groupby(['tcp', 'datarate', 'packetsize']
                    ).thr.agg(['mean', 'std']).reset_index()

thrs = []
for s in packetsizes:
    means_t = df_thr.loc[(df_thr['packetsize'] == s) & (df_thr['tcp'] == True)].sort_values(
        by=['datarate'], ascending=True)['mean']
    stds_t = df_thr.loc[(df_thr['packetsize'] == s) & (df_thr['tcp'] == True)].sort_values(
        by=['datarate'], ascending=True)['std']
    thrs.append({'means_t': list(means_t), 'stds_t': list(stds_t)})

    means_t = df_thr.loc[(df_thr['packetsize'] == s) & (df_thr['tcp'] == False)].sort_values(
        by=['datarate'], ascending=True)['mean']
    stds_t = df_thr.loc[(df_thr['packetsize'] == s) & (df_thr['tcp'] == False)].sort_values(
        by=['datarate'], ascending=True)['std']
    thrs.append({'means_t': list(means_t), 'stds_t': list(stds_t)})

# 9 degrees of freedom, 95%
eta = 2.262
n = 10

fig, axes = plt.subplots(1, 2, figsize=(15, 6))
fig.suptitle("Throughput", fontsize=20)
for i in range(len(thrs)):
    axes[i % 2].set_title('TCP' if i == 0 else 'UDP')
    axes[i % 2].plot(datarates, thrs[i]
                     ['means_t'], color='blue')
    axes[i % 2].fill_between(datarates, y1=np.array(thrs[i]['means_t']) - eta/math.sqrt(n) * np.array(
        thrs[i]['stds_t']), y2=np.array(thrs[i]['means_t']) + eta/math.sqrt(n) * np.array(thrs[i]['stds_t']), color='blue', alpha=0.2)
    axes[i % 2].set_xlim([0, 110])
    axes[i % 2].set_ylim([0, 110])
    axes[i % 2].set_xlabel('Offered [Mbps]')
    axes[i % 2].set_ylabel('Throughput [Mbps]')
    #axes[i % 2].legend(loc="upper left")
plt.savefig('throughput.png')
