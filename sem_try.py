def main():
    import sem
    import os

    script = 'project_log_dist_diff'
    ns_path = os.path.join(os.path.dirname(
        os.path.realpath(__file__)), 'ns-3-dev')
    campaign_dir = "./project_campaign_diff"

    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
                                       runner_type='ParallelRunner',
                                       check_repo=False,
                                       overwrite=False,
                                       skip_configuration=False)

    param_combinations = {
        'rtscts': [True, False],
        'datarate': [1, 2.5, 5, 7.5, 10, 15, 20],
        'packetsize': [1000]
    }

    campaign.run_missing_simulations(
        sem.list_param_combinations(param_combinations), runs=10)


if __name__ == '__main__':
    main()
