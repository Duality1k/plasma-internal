#pragma once

namespace rbx
{
	class Scheduler
	{
	private:
		std::uintptr_t scheduler;

	public:
		explicit Scheduler();
		std::vector<std::uintptr_t> getJobs();
		std::uintptr_t getJobByName(std::string name);
	};

	Scheduler::Scheduler() : scheduler(rbx::funcs::lua::get_task_scheduler()) {}

	std::vector<std::uintptr_t> Scheduler::getJobs()
	{
		std::vector<std::uintptr_t> jobs;
		std::uintptr_t* current_job = memory::read<std::uintptr_t*>(this->scheduler + rbx::offsets::job_start);
		do {
			jobs.push_back(*current_job);
			current_job += 2;
		} while (current_job != memory::read<std::uintptr_t*>(this->scheduler + rbx::offsets::job_end));

		return jobs;
	}

	std::uintptr_t Scheduler::getJobByName(std::string name) {

		auto jobs = this->getJobs();

		for (std::uintptr_t& job : jobs)
			if (memory::read<std::string>(job + rbx::offsets::job_name) == name)
				return job;
		
		return NULL;
	}
}