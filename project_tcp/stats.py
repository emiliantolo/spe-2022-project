import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import json
import math

f = open('./project_campaign/project_campaign.json', 'r')
data = json.load(f)

df = pd.DataFrame(columns=['id', 'rtscts', 'hidden', 'tcp', 'datarate', 'packetsize',
                  'verbose', 'rng', 'thr', 'rtr'])

for i in data['results']:
    id = data['results'][i]['meta']['id']
    file = open('./project_campaign/data/' + id + '/stdout', 'r')
    lines = file.readlines()
    thr1 = float(lines[6].split(':')[1].split('Mbps')[0].strip())
    thr2 = float(lines[14].split(':')[1].split('Mbps')[0].strip())
    count_file = open('./project_campaign/data/' +
                      id + '/count.txt', 'r')
    count_lines = count_file.readlines()
    retr = int(count_lines[0]) + int(count_lines[1])
    df.loc[len(df)] = [id] + list(data['results'][i]
                                  ['params'].values()) + [thr1 + thr2, retr]

df.to_csv('stats.csv', sep='\t')

datarates = np.sort(df['datarate'].unique()) * 2

df_thr = df.groupby(['rtscts', 'datarate', 'hidden', 'tcp']
                    ).thr.agg(['mean', 'std']).reset_index()
df_rtr = df.groupby(['rtscts', 'datarate', 'hidden', 'tcp']
                    ).rtr.agg(['mean', 'std']).reset_index()

thrs = []
rtrs = []
means_t = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
thrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})

means_t = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
rtrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})


means_t = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_thr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
thrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})

means_t = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_rtr.loc[(df_thr['hidden'] == True) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
rtrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})


means_t = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
thrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})

means_t = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == True) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
rtrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})


means_t = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_thr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_thr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
thrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})

means_t = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
stds_t = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
means_f = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
stds_f = df_rtr.loc[(df_thr['hidden'] == False) & (df_thr['tcp'] == False) & (
    df_rtr['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
rtrs.append({'means_t': list(means_t), 'stds_t': list(stds_t),
            'means_f': list(means_f), 'stds_f': list(stds_f)})

# 9 degrees of freedom, 95%
eta = 2.262
n = 10

title = ['TCP hidden', 'UDP hidden', 'TCP exposed', 'UDP exposed']

fig, axes = plt.subplots(2, 2, figsize=(15, 10))
fig.suptitle("Throughput", fontsize=20)
for i in range(4):
    axes[i // 2][i % 2].set_title(title[i])
    axes[i // 2][i % 2].plot(datarates, thrs[i]
                             ['means_t'], color='blue', label='RTS/CTS')
    axes[i // 2][i % 2].fill_between(datarates, y1=np.array(thrs[i]['means_t']) - eta/math.sqrt(n) * np.array(
        thrs[0]['stds_t']), y2=np.array(thrs[i]['means_t']) + eta/math.sqrt(n) * np.array(thrs[i]['stds_t']), color='blue', alpha=0.2)
    axes[i // 2][i % 2].plot(datarates, thrs[i]
                             ['means_f'], color='red', label='No RTS/CTS')
    axes[i // 2][i % 2].fill_between(datarates, y1=np.array(thrs[i]['means_f']) - eta/math.sqrt(n) * np.array(
        thrs[0]['stds_f']), y2=np.array(thrs[i]['means_f']) + eta/math.sqrt(n) * np.array(thrs[i]['stds_f']), color='red', alpha=0.2)
    axes[i // 2][i % 2].set_xlabel('Offered [Mbps]')
    axes[i // 2][i % 2].set_ylabel('Throughput [Mbps]')
    axes[i // 2][i % 2].legend(loc="upper left")
plt.savefig('throughput.png')

fig, axes = plt.subplots(2, 2, figsize=(15, 10))
fig.suptitle("Retransmission", fontsize=20)
for i in range(4):
    axes[i // 2][i % 2].set_title(title[i])
    axes[i // 2][i % 2].plot(datarates, rtrs[i]
                             ['means_t'], color='blue', label='RTS/CTS')
    axes[i // 2][i % 2].fill_between(datarates, y1=np.array(rtrs[i]['means_t']) - eta/math.sqrt(n) * np.array(
        rtrs[i]['stds_t']), y2=np.array(rtrs[i]['means_t']) + eta/math.sqrt(n) * np.array(rtrs[i]['stds_t']), color='blue', alpha=0.2)
    axes[i // 2][i % 2].plot(datarates, rtrs[i]
                             ['means_f'], color='red', label='No RTS/CTS')
    axes[i // 2][i % 2].fill_between(datarates, y1=np.array(rtrs[i]['means_f']) - eta/math.sqrt(n) * np.array(
        rtrs[i]['stds_f']), y2=np.array(rtrs[i]['means_f']) + eta/math.sqrt(n) * np.array(rtrs[i]['stds_f']), color='red', alpha=0.2)
    axes[i // 2][i % 2].set_xlabel('Offered [Mbps]')
    axes[i // 2][i % 2].set_ylabel('# Retry')
    axes[i // 2][i % 2].legend(loc="upper left")
plt.savefig('retry.png')
