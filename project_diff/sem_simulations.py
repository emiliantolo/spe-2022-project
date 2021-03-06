def main():
    import sem
    import os

    script = 'project_diff'
    ns_path = os.path.join(os.path.dirname(
        os.path.realpath(__file__)), '..', 'ns-3-dev')
    campaign_dir = "./project_campaign"

    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
                                       runner_type='ParallelRunner',
                                       check_repo=False,
                                       overwrite=False)

    param_combinations = {
        'rtscts': [True, False],
        'hidden': [True, False],
        'datarate': [1, 2.5, 5, 7.5, 10, 12.5, 15, 17.5, 20],
        'packetsize': [1000]
    }

    campaign.run_missing_simulations(
        sem.list_param_combinations(param_combinations), runs=10)


if __name__ == '__main__':
    main()
