
//#define LARSDEBUG
#ifdef LARSDEBUG
#include <iostream>
#define LARSLOG(value) { std::cout << "log: " << value << std::endl;  }
#else
#define LARSLOG(value)
#endif
#define LARSLOGV(value) LARSLOG(#value << " = " << value)
#define LARSLOGT() LARSLOG("reached " << ::app::log_helper::file_name(__FILE__) << ":" << __LINE__ << " with function \"" << __PRETTY_FUNCTION__ << "\"" )
