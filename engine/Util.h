#ifndef _ENGINE_UTIL_H_
#define _ENGINE_UTIL_H_

enum Status {
	Success,
	KeyNotFound,
	KeyExists,
	KeyRemove,
	TableFull,
	IOError,
	UnknownError
};

#define ASSERT(S) do{ Status res = S;if(res != Status::Success) return res;}while(0)

#endif