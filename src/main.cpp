#include "logging.h"
#include "object.h"

using namespace oemros;

using std::make_shared;
using std::dynamic_pointer_cast;
using std::cout;
using std::endl;

class branch : public object {
    public:
        strong_ptr<branch> strong_ref() { return OBJECT_AUTO_CAST(branch, object::shared_from_this()); }
        weak_ptr<branch> weak_ref() { return strong_ref(); }
        void totally() { cout << "totally" << endl; }
};

class leaf : public branch {
    public:
        template<typename... Args>
        static strong_ptr<leaf> make_shared(Args&&...args) {
            return std::make_shared<leaf>(args...);
        }
        strong_ptr<leaf> strong_ref() { return OBJECT_AUTO_CAST(leaf, object::strong_ref()); }
        weak_ptr<leaf> weak_ref() { return strong_ref(); }
        void blah() { cout << "blah" << endl; }
};

class leaf_leaf : public leaf {
    public:
        template<typename... Args>
        static strong_ptr<leaf_leaf> make_shared(Args&&...args) {
            return std::make_shared<leaf_leaf>(args...);
        }
        strong_ptr<leaf_leaf> strong_ref() { return OBJECT_AUTO_CAST(leaf_leaf, object::strong_ref()); }
        weak_ptr<leaf_leaf> weak_ref() { return strong_ref(); }
        void ohyeah() { cout << "ohyeah" << endl; }
};

void run() {
    auto thing = leaf::make_shared();
    auto downcast = dynamic_pointer_cast<object>(thing);
    auto upcast = dynamic_pointer_cast<leaf>(downcast);
    auto branch_cast = dynamic_pointer_cast<branch>(upcast);
    branch* branch_p = branch_cast.get();
    strong_ptr<branch> should_work = branch_p->strong_ref();

    auto ok = leaf_leaf::make_shared();

    ok->ohyeah();

    upcast->blah();
    should_work->totally();

    cout << thing.use_count() << endl;
    cout << downcast.use_count() << endl;
    cout << upcast.use_count() << endl;
    cout << branch_cast.use_count() << endl;
    cout << should_work.use_count() << endl;
    cout << endl;
}

int main(UNUSED int argc, UNUSED char ** argv) {
    run();
    return 0;
}
