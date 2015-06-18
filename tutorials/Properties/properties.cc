#include <SgtCore/Properties.h>

class Foo : public Sgt::HasProperties<Foo>
{
    public:
        SGT_PROPS_INIT(Foo);                                                               

        int four() const {return 4;}
        SGT_PROP_GET(fourProp, Foo, int, four);
};

class Bar : public Foo, public Sgt::HasProperties<Bar>
{
    public:
        SGT_PROPS_INIT(Bar);                                                               
        SGT_PROPS_INHERIT(Bar, Foo);          

        const int& x() const {return x_;}
        void setX(const int& x) {x_ = x;}
        SGT_PROP_GET_SET(xProp, Bar, const int&, x, setX);

    public:
        int x_{5};
};

int main()
{
    Bar bar;
    for (auto& p : bar.properties())
    {
        std::cout << "Property " << p.first << " : " << p.second->string(bar) << std::endl;
    }
    Sgt::PropertyBase& xProp = *bar.properties()["xProp"];
    std::cout << "xProp as string : " << xProp.string(bar) << std::endl;
    std::cout << "xProp as const int& : " << xProp.get<const int&>(bar) << std::endl;
    xProp.set<const int&>(bar, 345);
    std::cout << "bar.x() should now be 345 : " << bar.x() << std::endl;
    xProp.setFromString(bar, "678");
    std::cout << "bar.x() should now be 678 : " << bar.x() << std::endl;
};
