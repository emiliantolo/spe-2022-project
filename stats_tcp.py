import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import json
import math

f = open('./project_campaign_tcp/project_campaign_tcp.json', 'r')
data = json.load(f)

df = pd.DataFrame(columns=['id', 'rtscts', 'hidden', 'tcp', 'datarate', 'packetsize',
                  'verbose', 'rng', 'thr', 'rtr'])


# for i in data['results']:
#    rtscts = data['results'][i]['params']['rstcts']
#    datarate = data['results'][i]['params']['datarate']
#    packetsize = data['results'][i]['params']['packetsize']
#    key = str(rtscts) + '_' + str(datarate) + '_' + str(packetsize)
#    id = data['results'][i]['meta']['id']


for i in data['results']:
    id = data['results'][i]['meta']['id']
    file = open('./project_campaign_tcp/data/' + id + '/stdout', 'r')
    lines = file.readlines()
    thr1 = float(lines[6].split(':')[1].split('Mbps')[0].strip())
    thr2 = float(lines[14].split(':')[1].split('Mbps')[0].strip())
    count_file = open('./project_campaign_tcp/data/' +
                      id + '/count.txt', 'r')
    count_lines = count_file.readlines()
    retr = int(count_lines[0]) + int(count_lines[1])
    df.loc[len(df)] = [id] + list(data['results'][i]
                                  ['params'].values()) + [thr1 + thr2, retr]

df.to_csv('stats_tcp.csv', sep='\t')

datarates = np.sort(df['datarate'].unique()) * 2


df_thr = df.groupby(['rtscts', 'datarate', 'packetsize']
                    ).thr.agg(['mean', 'std']).reset_index()
df_rtr = df.groupby(['rtscts', 'datarate', 'packetsize']
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

fig, axes = plt.subplots(figsize=(15, 10))
for i in range(4):
    axes.set_title('Packet size: ')
    axes.plot(datarates, thrs[i]
              ['means_t'], color='blue', label='RTS/CTS')
    axes.fill_between(datarates, y1=np.array(thrs[i]['means_t']) - eta/math.sqrt(n) * np.array(
        thrs[0]['stds_t']), y2=np.array(thrs[i]['means_t']) + eta/math.sqrt(n) * np.array(thrs[i]['stds_t']), color='blue', alpha=0.2)
    axes.plot(datarates, thrs[i]
              ['means_f'], color='red', label='No RTS/CTS')
    axes.fill_between(datarates, y1=np.array(thrs[i]['means_f']) - eta/math.sqrt(n) * np.array(
        thrs[0]['stds_f']), y2=np.array(thrs[i]['means_f']) + eta/math.sqrt(n) * np.array(thrs[i]['stds_f']), color='red', alpha=0.2)
    #axes.set_xlim([0, 30])
    #axes.set_ylim([0, 30])
    axes.legend(loc="upper left")
plt.savefig('thr_tcp.png')

fig, axes = plt.subplots(1, 1, figsize=(15, 10))
axes.set_title('Packet size: ')
axes.plot(datarates, rtrs[0]
          ['means_t'], color='blue', label='RTS/CTS')
axes.fill_between(datarates, y1=np.array(rtrs[0]['means_t']) - eta/math.sqrt(n) * np.array(
    rtrs[0]['stds_t']), y2=np.array(rtrs[0]['means_t']) + eta/math.sqrt(n) * np.array(rtrs[0]['stds_t']), color='blue', alpha=0.2)
axes.plot(datarates, rtrs[0]
          ['means_f'], color='red', label='No RTS/CTS')
axes.fill_between(datarates, y1=np.array(rtrs[0]['means_f']) - eta/math.sqrt(n) * np.array(
    rtrs[0]['stds_f']), y2=np.array(rtrs[0]['means_f']) + eta/math.sqrt(n) * np.array(rtrs[0]['stds_f']), color='red', alpha=0.2)
#axes.set_xlim([0, 30])
#axes.set_ylim([0, 20000])
axes.legend(loc="upper left")
plt.savefig('rtr_tcp.png')
