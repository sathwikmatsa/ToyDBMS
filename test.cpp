#include "crud_cpp.h"
using namespace std;

vector <heapObject*> HeapObjects;
database Database;

int main(){
    create_table_in_database("Avengers");
    add_attribute("Name", true, "Avengers");
    add_attribute("Age", false, "Avengers");
    string s = "Unknown";
    add_constraint(0, "Age", "Avengers", &s);
    record r1{"Tony Stark", "30"};
    record r2{"Dr.Strange", "29"};
    add_record(r1, "Avengers");
    add_record(r2, "Avengers");
    print_database();
    garbageCollector();
    return 0;
}
