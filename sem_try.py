def main():
    import sem
    import os

    script = 'project'
    ns_path = os.path.join(os.path.dirname(
        os.path.realpath(__file__)), 'ns-3-dev')
    campaign_dir = "./project_campaign_tcp"

    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
                                       runner_type='ParallelRunner',
                                       check_repo=False,
                                       overwrite=False)

    param_combinations = {
        'rtscts': [True, False],
        'datarate': [1, 1.5, 2.5, 5, 10],
        'packetsize': [125, 250, 500, 1000, 1500, 2000]
    }

    campaign.run_missing_simulations(
        sem.list_param_combinations(param_combinations), runs=10)


if __name__ == '__main__':
    main()
