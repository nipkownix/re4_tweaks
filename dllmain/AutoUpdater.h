#pragma once

enum class UpdateStatus
{
	Available,
	InProgress,
	Success,
	Failed,
	Finished
};

void updateCheck();

struct AutoUpdate
{
	std::string version;
	std::string url;
	std::string description;

	void RenderUI();

	UpdateStatus UpdateStatus;
};

extern AutoUpdate updt;