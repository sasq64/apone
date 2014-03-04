
```C++
WebGetter::getURL("http://somehost/rpc/list_users", [](const vector<uint8_t> &data) {
	for(auto &j : JSon::parse(data)) {
		string name = j("name");
		LOGD("Got user '%s'", name);
	}
});
```