all: test

proto.ts: test.proto
	@mkdir -p cpp_out
	protoc $< --cpp_out=cpp_out/
	@touch $@

test: test.cpp proto.ts
	g++ -g -O2 --std=c++11 $< cpp_out/test.pb.cc -o $@ -Icpp_out -lprotobuf

clean:
	rm -rf test proto.ts cpp_out
