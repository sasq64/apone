
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
				LOGD("Curl done");
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

