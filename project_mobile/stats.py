import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import json
import math

f = open('./project_campaign/project_campaign.json', 'r')
data = json.load(f)

df = pd.DataFrame(columns=['id', 'rtscts', 'datarate', 'packetsize',
                  'maxrange', 'nstations', 'verbose', 'rng', 'thr', 'rtr'])

l = 0

for i in data['results']:
    id = data['results'][i]['meta']['id']
    file = open('./project_campaign/data/' + id + '/stdout', 'r')
    lines = file.readlines()

    thr = 0
    l = len(lines) // 8
    for j in range(l):
        thr += float(lines[(j * 8) + 6].split(':')[1].split('Mbps')[0].strip())
    count_file = open('./project_campaign/data/' +
                      id + '/count.txt', 'r')
    count_lines = count_file.readlines()
    retr = np.sum([int(c) for c in count_lines])
    df.loc[len(df)] = [id] + list(data['results'][i]
                                  ['params'].values()) + [thr, retr]

df.to_csv('stats_tcp.csv', sep='\t')

datarates = np.sort(df['datarate'].unique())
maxranges = np.sort(df['maxrange'].unique())
nstations = np.sort(df['nstations'].unique())

df_thr = df.groupby(['rtscts', 'datarate', 'packetsize', 'maxrange', 'nstations']
                    ).thr.agg(['mean', 'std']).reset_index()
df_rtr = df.groupby(['rtscts', 'datarate', 'packetsize', 'maxrange', 'nstations']
                    ).rtr.agg(['mean', 'std']).reset_index()

thrs = []
rtrs = []
for m in maxranges:
    for n in nstations:
        means_t = df_thr.loc[(df_thr['maxrange'] == m) & (df_thr['nstations'] == n) & (
            df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
        stds_t = df_thr.loc[(df_thr['maxrange'] == m) & (df_thr['nstations'] == n) & (
            df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
        means_f = df_thr.loc[(df_thr['maxrange'] == m) & (df_thr['nstations'] == n) & (
            df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
        stds_f = df_thr.loc[(df_thr['maxrange'] == m) & (df_thr['nstations'] == n) & (
            df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
        thrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
                    'means_f': list(means_f), 'stds_f': list(stds_f)})

        means_t = df_rtr.loc[(df_rtr['maxrange'] == m) & (df_rtr['nstations'] == n) & (
            df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
        stds_t = df_rtr.loc[(df_rtr['maxrange'] == m) & (df_rtr['nstations'] == n) & (
            df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
        means_f = df_rtr.loc[(df_rtr['maxrange'] == m) & (df_rtr['nstations'] == n) & (
            df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
        stds_f = df_rtr.loc[(df_rtr['maxrange'] == m) & (df_rtr['nstations'] == n) & (
            df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
        rtrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
                    'means_f': list(means_f), 'stds_f': list(stds_f)})

# 9 degrees of freedom, 95%
eta = 2.262
n = 10

l = int(len(thrs) / 3 if len(thrs) % 3 == 0 else (len(thrs) // 3 + 1))

fig, axes = plt.subplots(l, 3, figsize=(22.5, l * 6))
fig.suptitle("Throughput", fontsize=20)
for i in range(len(thrs)):
    axes[i // 3][i % 3].set_title('Range: ' + str(maxranges[i // len(
        maxranges)]) + ' m - ' + str(nstations[i % len(nstations)]) + ' mobile nodes')
    axes[i // 3][i % 3].plot(datarates * (nstations[i % len(nstations)]), thrs[i]
                             ['means_t'], color='blue', label='RTS/CTS')
    axes[i // 3][i % 3].fill_between(datarates * (nstations[i % len(nstations)]), y1=np.array(thrs[i]['means_t']) - eta/math.sqrt(n) * np.array(
        thrs[i]['stds_t']), y2=np.array(thrs[i]['means_t']) + eta/math.sqrt(n) * np.array(thrs[i]['stds_t']), color='blue', alpha=0.2)
    axes[i // 3][i % 3].plot(datarates * (nstations[i % len(nstations)]), thrs[i]
                             ['means_f'], color='red', label='No RTS/CTS')
    axes[i // 3][i % 3].fill_between(datarates * (nstations[i % len(nstations)]), y1=np.array(thrs[i]['means_f']) - eta/math.sqrt(n) * np.array(
        thrs[i]['stds_f']), y2=np.array(thrs[i]['means_f']) + eta/math.sqrt(n) * np.array(thrs[i]['stds_f']), color='red', alpha=0.2)
    axes[i // 3][i % 3].set_xlim([0, 85])
    axes[i // 3][i % 3].set_ylim([0, 40])
    axes[i // 3][i % 3].set_xlabel('Offered [Mbps]')
    axes[i // 3][i % 3].set_ylabel('Throughput [Mbps]')
    axes[i // 3][i % 3].legend(loc="upper left")
plt.savefig('throughput.png')

l = int(len(rtrs) / 3 if len(rtrs) % 3 == 0 else (len(rtrs) // 3 + 1))

fig, axes = plt.subplots(l, 3, figsize=(22.5, l * 6))
fig.suptitle("Retransmission", fontsize=20)
for i in range(len(rtrs)):
    axes[i // 3][i % 3].set_title('Range: ' + str(maxranges[i // len(
        maxranges)]) + ' m - ' + str(nstations[i % len(nstations)]) + ' mobile nodes')
    axes[i // 3][i % 3].plot(datarates * (nstations[i % len(nstations)]), rtrs[i]
                             ['means_t'], color='blue', label='RTS/CTS')
    axes[i // 3][i % 3].fill_between(datarates * (nstations[i % len(nstations)]), y1=np.array(rtrs[i]['means_t']) - eta/math.sqrt(n) * np.array(
        rtrs[i]['stds_t']), y2=np.array(rtrs[i]['means_t']) + eta/math.sqrt(n) * np.array(rtrs[i]['stds_t']), color='blue', alpha=0.2)
    axes[i // 3][i % 3].plot(datarates * (nstations[i % len(nstations)]), rtrs[i]
                             ['means_f'], color='red', label='No RTS/CTS')
    axes[i // 3][i % 3].fill_between(datarates * (nstations[i % len(nstations)]), y1=np.array(rtrs[i]['means_f']) - eta/math.sqrt(n) * np.array(
        rtrs[i]['stds_f']), y2=np.array(rtrs[i]['means_f']) + eta/math.sqrt(n) * np.array(rtrs[i]['stds_f']), color='red', alpha=0.2)
    axes[i // 3][i % 3].set_xlim([0, 85])
    axes[i // 3][i % 3].set_ylim([0, 10000])
    axes[i // 3][i % 3].set_xlabel('Offered [Mbps]')
    axes[i // 3][i % 3].set_ylabel('# Retry')
    axes[i // 3][i % 3].legend(loc="upper left")
plt.savefig('retry.png')
