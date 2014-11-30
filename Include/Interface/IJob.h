#pragma once
#include <list>
/**
struct ICondition {
	virtual void Notify() = 0;
	virtual void Wait() = 0;
};


struct IJob {
	
	virtual ~IJob() {}
	
	virtual void OnRun() = 0;

	virtual void AddDependency(IJob *) = 0;

	virtual void OnComplete() {
		if (m_JoinedJob != nullptr) {
			JobList list = m_JoinedJob->GetDependencies();
			list.remove(this);
			m_JoinedJob->GetPreqCondition()->Notify();
		}
	}

	virtual void Execute() = 0;

	void SetJoinedJob(JoinedJob * joinedJob) {
		m_JoinedJob = joinedJob;
	}

	using JobList = std::list<IJob*>;

protected:
	JoinedJob * m_JoinedJob;
};


struct JoinedJob : IJob {
	virtual ~JoinedJob() {}
	virtual void OnRun() = 0;

	void AddDependency(IJob * job) override {
		job->SetJoinedJob(this);
		m_DependJobs.push_back(job);
	}

	void Execute() override {
		while (m_PreqCondition!=nullptr && !m_DependJobs.empty()) {
			m_PreqCondition->Wait();
		}
		OnRun();
	}

	ICondition * GetPreqCondition() {
		return m_PreqCondition;
	}

	JobList & GetDependencies();

private:
	JobList			m_DependJobs;
	ICondition *	m_PreqCondition;
};

struct SerialJob : IJob {

	void AddDependency(IJob * job) override {
		m_DependJobs.push_back(job);
	}

	void Execute() override {
		while (!m_DependJobs.empty()) {
			IJob* job = m_DependJobs.back();
			job->OnRun();
			m_DependJobs.remove(job);
		}
		OnRun();
	}

private:
	JobList			m_DependJobs;
};
**/