
#include "web.h"

namespace webutils {
	__thread CURLM *curlm = nullptr;
	int poll() {

		auto *cm = curlm;
		CURLMcode rc = CURLM_CALL_MULTI_PERFORM;
		int count;
		while(rc == CURLM_CALL_MULTI_PERFORM)
			rc = curl_multi_perform(cm, &count);

		CURLMsg *msg;
		while((msg = curl_multi_info_read(cm, &count))	) {
			if(msg->msg == CURLMSG_DONE) {
				LOGD("Curl %p done");
				std::shared_ptr<BaseJob> *job;
				curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, (char**)&job);
				if(job) {
					(*job)->onDone();
				} else
					LOGW("No attached job ?!");

			}
		}

		return 0;
	}

} // namespace webutils

#ifdef UNIT_TEST

#include "catch.hpp"
#include "sinks.h"
#include <coreutils/file.h>
#include <atomic>

using namespace utils;

TEST_CASE("webutils::web", "Web gets") {

	std::string res = webutils::get<std::string>("http://localhost:8000/CppTips.html");
	REQUIRE(res != "");
	bool done = false;
	webutils::get<utils::File>("http://localhost:8000/style.css", utils::File::getTempFile()).
		onDone([&](utils::File &res) {
			LOGD("RES: %s", res.getName());
			done = true;
			REQUIRE(res.getSize() == 4762);
		});

	while(!done) {
		utils::sleepms(5);
		webutils::poll();
	}
}

TEST_CASE("webutils::web2", "Web thread test")
{
	std::vector<std::thread> threads(100);
	std::atomic<int> done(0);
	for(auto &t : threads) {
		t = std::thread([&] {
			auto w = webutils::get<utils::File>("http://localhost:8000/style.css", utils::File::getTempFile()).
				onDone([&](utils::File &res) {
					done++;
				});
			w.finish();
		});
		utils::sleepms(1);
	}
	for(auto &t : threads)
		t.join();
	REQUIRE(done.load() == threads.size());
}

#endif
