#pragma once


class LoopMode final
{
public:
    LoopMode() = delete;
    
    enum class Mode { none, fade, trigger, gap };
    static constexpr char const *labels[] = { "None", "Fade", "Trigger", "Gap", nullptr };
    
    operator Mode() const { return std::get<0>(assocValue); }
    float value() const { return std::get<1>(assocValue); }
    bool operator==(const LoopMode& rhs) const { return assocValue == rhs.assocValue; }
    
    static LoopMode none() { return LoopMode({ Mode::none, 0.0 }); }
    static LoopMode fade(const float duration) { return LoopMode({ Mode::fade, duration }); }
    static LoopMode trigger(const float rate) { return LoopMode({ Mode::trigger, rate }); }
    static LoopMode gap(const float length) { return LoopMode({ Mode::gap, length }); }
    
private:
    using AssociatedValue = std::tuple<Mode, float>;
    LoopMode(AssociatedValue av) : assocValue(av) {}
    AssociatedValue assocValue;
};

#pragma mark -

class PlaybackOrder final
{
public:
    PlaybackOrder() = delete;
    
    enum class Order { ordinal, shuffle, random };
    static constexpr char const *labels[] = { "Ordinal", "Shuffle", "Random", nullptr };
    
    operator Order() const { return order; }
    bool operator==(const PlaybackOrder& rhs) const { return order == rhs.order; }
    
    static PlaybackOrder ordinal() { return { Order::ordinal }; }
    static PlaybackOrder shuffle() { return { Order::shuffle }; }
    static PlaybackOrder random() { return { Order::random }; }
    
private:
    PlaybackOrder(Order o) : order(o) {}
    Order order;
};

#pragma mark -

template <typename I>
size_t index_of(I first, I last, typename std::iterator_traits<I>::value_type val)
{
    size_t i = 0;
    while (first != last && *first != val)
        ++first; ++i;
    return i;
}

