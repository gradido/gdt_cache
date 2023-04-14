#include "MultithreadContainer.h"

#include <thread>
#include <chrono>

void MultithreadContainer::lock(const char* stackDetails/* = nullptr*/)
{
	using namespace std::chrono_literals;
	const static char* functionName = "MultithreadContainer::lock";
			
	uint8_t exitCounter = 5;
	while (!mWorkMutex.try_lock() && exitCounter > 0) {
		std::this_thread::sleep_for(100ms);
		exitCounter--;
	}
	if (exitCounter > 0) {
		if (stackDetails) {
			mLastSucceededLock = stackDetails;
		}
	}
	else {
		throw MultithreadContainerLockTimeoutException(mLastSucceededLock.data(), stackDetails);
	}			
}

bool MultithreadContainer::tryLock()
{
	return mWorkMutex.try_lock();
}

// ************ Exception **************
MultithreadContainerLockTimeoutException::MultithreadContainerLockTimeoutException(const char* lastSucceedLock, const char* stackDetails)
	: GradidoBlockchainException("MultithreadContainer lock timeout")
{
	if (lastSucceedLock) {
		mLastSucceedLock = lastSucceedLock;
	}
	if (stackDetails) {
		mStackDetails = stackDetails;
	}
}

std::string MultithreadContainerLockTimeoutException::getFullString() const
{
	if (mLastSucceedLock.size() || mStackDetails.size()) {
		size_t resultSize = 0;
		if (mLastSucceedLock.size()) {
			resultSize += mLastSucceedLock.size() + 14;
		}
		if (mStackDetails.size()) {
			resultSize += mStackDetails.size() + 10;
		}
		std::string result;
		result.reserve(resultSize);
		if (mLastSucceedLock.size()) {
			result += "last succeed: " + mLastSucceedLock;
		}
		if (mStackDetails.size()) {
			result += ", stack: " + mStackDetails;
		}
		return result;
	}
	else {
		return what();
	}			
}
