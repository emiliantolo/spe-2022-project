def main():
    import sem
    import os

    script = 'project_csma'
    ns_path = os.path.join(os.path.dirname(
        os.path.realpath(__file__)), '..', 'ns-3-dev')
    campaign_dir = "./project_campaign"

    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
                                       runner_type='ParallelRunner',
                                       check_repo=False,
                                       overwrite=False)

    param_combinations = {
        'tcp': [True, False],
        'datarate': [5, 10, 15, 20, 25, 30, 35, 40, 45, 50],
        'packetsize': [1000]
    }

    campaign.run_missing_simulations(
        sem.list_param_combinations(param_combinations), runs=10)


if __name__ == '__main__':
    main()
