#include <string>



class StorageEngine {
public:

virtual void set(const std::string& key, const std::string& value) = 0;

virtual std::string get(const std::string& key) = 0;

virtual void del(const std::string& key) = 0;

virtual ~StorageEngine() = default;


 };