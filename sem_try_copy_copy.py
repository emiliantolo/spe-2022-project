def main():
    import sem
    import os

    script = 'project_exposed_four_copy'
    ns_path = os.path.join(os.path.dirname(
        os.path.realpath(__file__)), 'ns-3-dev')
    campaign_dir = "./project_campaign_aaa"

    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
                                       runner_type='ParallelRunner',
                                       check_repo=False,
                                       overwrite=False)

    param_combinations = {

    }

    campaign.run_missing_simulations(
        sem.list_param_combinations(param_combinations), runs=30)


if __name__ == '__main__':
    main()
