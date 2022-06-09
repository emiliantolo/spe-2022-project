def main():
    import sem
    import os

    script = 'project_2'
    ns_path = os.path.join(os.path.dirname(
        os.path.realpath(__file__)), 'ns-3-dev')
    campaign_dir = "./project_campaign_2"

    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
                                       runner_type='ParallelRunner',
                                       check_repo=False,
                                       overwrite=False)

    param_combinations = {
        'rtscts': [True, False],
        'standard': [0, 1, 2],
        'datarate': ['1', '2', '4', '8', '16']
    }

    campaign.run_missing_simulations(
        sem.list_param_combinations(param_combinations), runs=1)


if __name__ == '__main__':
    main()
