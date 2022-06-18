import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import json
import math

f = open('./project_campaign_diff/project_campaign_diff.json', 'r')
data = json.load(f)

df = pd.DataFrame(columns=['id', 'rtscts', 'datarate', 'packetsize',
                  'verbose', 'rng', 'thr1',  'rtr1', 'thr2', 'rtr2'])


# for i in data['results']:
#    rtscts = data['results'][i]['params']['rstcts']
#    datarate = data['results'][i]['params']['datarate']
#    packetsize = data['results'][i]['params']['packetsize']
#    key = str(rtscts) + '_' + str(datarate) + '_' + str(packetsize)
#    id = data['results'][i]['meta']['id']


for i in data['results']:
    id = data['results'][i]['meta']['id']
    file = open('./project_campaign_diff/data/' + id + '/stdout', 'r')
    lines = file.readlines()
    thr1 = float(lines[6].split(':')[1].split('Mbps')[0].strip())
    thr2 = float(lines[14].split(':')[1].split('Mbps')[0].strip())
    count_file = open('./project_campaign_diff/data/' +
                      id + '/count.txt', 'r')
    count_lines = count_file.readlines()
    retr1 = int(count_lines[0])
    retr2 = int(count_lines[1])
    df.loc[len(df)] = [id] + list(data['results'][i]
                                  ['params'].values()) + [thr1, retr1, thr2, retr2]

df.to_csv('stats_tcp.csv', sep='\t')

packetsizes = np.sort(df['packetsize'].unique())
datarates = np.sort(df['datarate'].unique())


df_thr1 = df.groupby(['rtscts', 'datarate', 'packetsize']
                     ).thr1.agg(['mean', 'std']).reset_index()
df_rtr1 = df.groupby(['rtscts', 'datarate', 'packetsize']
                     ).rtr1.agg(['mean', 'std']).reset_index()
df_thr2 = df.groupby(['rtscts', 'datarate', 'packetsize']
                     ).thr2.agg(['mean', 'std']).reset_index()
df_rtr2 = df.groupby(['rtscts', 'datarate', 'packetsize']
                     ).rtr2.agg(['mean', 'std']).reset_index()

thrs = []
rtrs = []
for s in packetsizes:
    means1_t = df_thr1.loc[(df_thr1['packetsize'] == s) & (
        df_thr1['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
    stds1_t = df_thr1.loc[(df_thr1['packetsize'] == s) & (
        df_thr1['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
    means1_f = df_thr1.loc[(df_thr1['packetsize'] == s) & (
        df_thr1['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
    stds1_f = df_thr1.loc[(df_thr1['packetsize'] == s) & (
        df_thr1['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
    means2_t = df_thr2.loc[(df_thr2['packetsize'] == s) & (
        df_thr2['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
    stds2_t = df_thr2.loc[(df_thr2['packetsize'] == s) & (
        df_thr2['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
    means2_f = df_thr2.loc[(df_thr2['packetsize'] == s) & (
        df_thr2['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
    stds2_f = df_thr2.loc[(df_thr2['packetsize'] == s) & (
        df_thr2['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
    thrs.append({'means1_t': list(means1_t), 'stds1_t': list(stds1_t), 'means1_f': list(means1_f), 'stds1_f': list(
        stds1_f), 'means2_t': list(means2_t), 'stds2_t': list(stds2_t), 'means2_f': list(means2_f), 'stds2_f': list(stds2_f)})

    means1_t = df_rtr1.loc[(df_rtr1['packetsize'] == s) & (
        df_rtr1['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
    stds1_t = df_rtr1.loc[(df_rtr1['packetsize'] == s) & (
        df_rtr1['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
    means1_f = df_rtr1.loc[(df_rtr1['packetsize'] == s) & (
        df_rtr1['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
    stds1_f = df_rtr1.loc[(df_rtr1['packetsize'] == s) & (
        df_rtr1['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
    means2_t = df_rtr2.loc[(df_rtr2['packetsize'] == s) & (
        df_rtr2['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['mean']
    stds2_t = df_rtr2.loc[(df_rtr2['packetsize'] == s) & (
        df_rtr2['rtscts'] == True)].sort_values(by=['datarate'], ascending=True)['std']
    means2_f = df_rtr2.loc[(df_rtr2['packetsize'] == s) & (
        df_rtr2['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['mean']
    stds2_f = df_rtr2.loc[(df_rtr2['packetsize'] == s) & (
        df_rtr2['rtscts'] == False)].sort_values(by=['datarate'], ascending=True)['std']
    rtrs.append({'means1_t': list(means1_t), 'stds1_t': list(stds1_t),
                 'means1_f': list(means1_f), 'stds1_f': list(stds1_f), 'means2_t': list(means2_t), 'stds2_t': list(stds2_t),
                 'means2_f': list(means2_f), 'stds2_f': list(stds2_f)})

# 9 degrees of freedom, 95%
eta = 2.262
n = 10

fig, axes = plt.subplots(figsize=(15, 10))
axes.set_title('Packet size: ' + str(packetsizes[0]))
axes.plot(datarates, thrs[0]
          ['means1_t'], color='blue', label='1 RTS/CTS')
axes.fill_between(datarates, y1=np.array(thrs[0]['means1_t']) - eta/math.sqrt(n) * np.array(
    thrs[0]['stds1_t']), y2=np.array(thrs[0]['means1_t']) + eta/math.sqrt(n) * np.array(thrs[0]['stds1_t']), color='blue', alpha=0.2)
axes.plot(datarates, thrs[0]
          ['means1_f'], color='red', label='1 No RTS/CTS')
axes.fill_between(datarates, y1=np.array(thrs[0]['means1_f']) - eta/math.sqrt(n) * np.array(
    thrs[0]['stds1_f']), y2=np.array(thrs[0]['means1_f']) + eta/math.sqrt(n) * np.array(thrs[0]['stds1_f']), color='red', alpha=0.2)
axes.plot(datarates, thrs[0]
          ['means2_t'], color='cyan', label='2 RTS/CTS')
axes.fill_between(datarates, y1=np.array(thrs[0]['means2_t']) - eta/math.sqrt(n) * np.array(
    thrs[0]['stds2_t']), y2=np.array(thrs[0]['means2_t']) + eta/math.sqrt(n) * np.array(thrs[0]['stds2_t']), color='cyan', alpha=0.2)
axes.plot(datarates, thrs[0]
          ['means2_f'], color='magenta', label='2 No RTS/CTS')
axes.fill_between(datarates, y1=np.array(thrs[0]['means2_f']) - eta/math.sqrt(n) * np.array(
    thrs[0]['stds2_f']), y2=np.array(thrs[0]['means2_f']) + eta/math.sqrt(n) * np.array(thrs[0]['stds2_f']), color='magenta', alpha=0.2)
#axes.set_xlim([0, 30])
#axes.set_ylim([0, 30])
axes.legend(loc="upper left")
plt.savefig('thr_diff.png')

fig, axes = plt.subplots(1, 1, figsize=(15, 10))
axes.set_title('Packet size: ' + str(packetsizes[0]))
axes.plot(datarates, rtrs[0]
          ['means1_t'], color='blue', label='1 RTS/CTS')
axes.fill_between(datarates, y1=np.array(rtrs[0]['means1_t']) - eta/math.sqrt(n) * np.array(
    rtrs[0]['stds1_t']), y2=np.array(rtrs[0]['means1_t']) + eta/math.sqrt(n) * np.array(rtrs[0]['stds1_t']), color='blue', alpha=0.2)
axes.plot(datarates, rtrs[0]
          ['means1_f'], color='red', label='1 No RTS/CTS')
axes.fill_between(datarates, y1=np.array(rtrs[0]['means1_f']) - eta/math.sqrt(n) * np.array(
    rtrs[0]['stds1_f']), y2=np.array(rtrs[0]['means1_f']) + eta/math.sqrt(n) * np.array(rtrs[0]['stds1_f']), color='red', alpha=0.2)
axes.plot(datarates, rtrs[0]
          ['means2_t'], color='cyan', label='2 RTS/CTS')
axes.fill_between(datarates, y1=np.array(rtrs[0]['means2_t']) - eta/math.sqrt(n) * np.array(
    rtrs[0]['stds2_t']), y2=np.array(rtrs[0]['means2_t']) + eta/math.sqrt(n) * np.array(rtrs[0]['stds2_t']), color='cyan', alpha=0.2)
axes.plot(datarates, rtrs[0]
          ['means2_f'], color='magenta', label='2 No RTS/CTS')
axes.fill_between(datarates, y1=np.array(rtrs[0]['means2_f']) - eta/math.sqrt(n) * np.array(
    rtrs[0]['stds2_f']), y2=np.array(rtrs[0]['means2_f']) + eta/math.sqrt(n) * np.array(rtrs[0]['stds2_f']), color='magenta', alpha=0.2)
#axes.set_xlim([0, 30])
#axes.set_ylim([0, 20000])
axes.legend(loc="upper left")
plt.savefig('rtr_diff.png')
