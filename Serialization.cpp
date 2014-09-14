#include <cassert>
#include <fstream>
#include <type_traits>

template<typename...>
using void_t = void;

template<bool Condition, typename T>
using enable_if_t = typename std::enable_if<Condition, T>::type;

template<typename T, typename = void>
struct is_complete : std::false_type{};

template<typename T>
struct is_complete<T, void_t<decltype(sizeof(T))>> : std::true_type{};

template<typename T, typename S, typename = void>
struct has_serialize : std::false_type
{
    static_assert(is_complete<T>::value, "Incomplete type");
};

template<typename T, typename S>
struct has_serialize<T, S, void_t<decltype(std::declval<T&>().Serialize(std::declval<S&>()))>> : std::true_type{};

template<typename T>
enable_if_t<std::is_fundamental<T>::value, void> Serialize(std::ostream& stream, T value)
{
    stream.write(reinterpret_cast<char*>(&value), sizeof(T));
}

template<typename T>
enable_if_t<std::is_fundamental<T>::value, void> Serialize(std::istream& stream, T& value)
{
    stream.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template<typename S, typename T>
enable_if_t<has_serialize<T, S>::value, void> Serialize(S& stream, T& value)
{
    value.Serialize(stream);
}

struct Point
{
    Point() = default;
    Point(float _x, float _y) : x(_x), y(_y){}
    

    float x = 0;
    float y = 0;
};

template<typename Stream>
void Serialize(Stream& stream, Point& value)
{
    Serialize(stream, value.x);
    Serialize(stream, value.y);
}

struct Gun
{
    Gun() = default;
    Gun(int _damage) : damage(_damage){}

    template<typename StreamType>
    void Serialize(StreamType& stream)
    {
        ::Serialize(stream, damage);
    }
    
    int damage = 10;
};

struct Player
{
    Player() = default;
    Player(int _hp, const Point& _position) : hp(_hp), position(_position){}

    template<typename StreamType>
    void Serialize(StreamType& stream)
    {
        ::Serialize(stream, hp);
        ::Serialize(stream, position);
        ::Serialize(stream, gun);
    }
    
    int hp = 100;
    Point position;
    Gun gun;
};

int main(int argc, const char * argv[])
{
    {
        Player somePlayer(9001, {123.f, 321.f});
        std::ofstream stream("test", std::ios::binary);
        Serialize(stream, somePlayer);
    }

    {
        Player somePlayer;
        std::ifstream stream("test", std::ios::binary);

        Serialize(stream, somePlayer);
        assert(somePlayer.hp == 9001 && "Ops! My code sample is not working...");
        assert(somePlayer.position.x == 123.f && "Ops! My code sample is not working...");
        assert(somePlayer.position.y == 321.f && "Ops! My code sample is not working...");
    }
}

