WEBUTILS
========

```C++
webutils::get<string>("http://somehost/rpc/list_users").onDone([](const string &data) {
	for(auto &j : JSon::parse(data)) {
		string name = j("name");
		LOGD("Got user '%s'", name);
	}
});
```
