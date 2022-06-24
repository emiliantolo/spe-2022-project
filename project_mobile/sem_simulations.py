def main():
    import sem
    import os

    script = 'project_mobile'
    ns_path = os.path.join(os.path.dirname(
        os.path.realpath(__file__)), '..', 'ns-3-dev')
    campaign_dir = "./project_campaign"

    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
                                       runner_type='ParallelRunner',
                                       check_repo=False,
                                       overwrite=False)

    param_combinations = {
        'rtscts': [True, False],
        'datarate': [2.5, 5, 10, 20],
        'packetsize': [1000],
        'maxrange': [25.0, 50.0, 125.0],
        'nstations': [2, 3, 4]
    }

    campaign.run_missing_simulations(
        sem.list_param_combinations(param_combinations), runs=10)


if __name__ == '__main__':
    main()
