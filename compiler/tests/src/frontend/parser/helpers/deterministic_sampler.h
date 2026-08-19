#pragma once

#include <concepts>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <random>

namespace valuascript::compiler::test
{
    template <typename Container>
    concept SampleableContainer = std::ranges::random_access_range<Container> &&
        std::ranges::sized_range<Container>;

    class DeterministicSampler
    {
    public:
        template <typename K>
        static size_t hash_single(const K& key)
        {
            using DecayedK = std::decay_t<K>;
            if constexpr (std::is_convertible_v<DecayedK, std::string_view>)
            {
                return std::hash<std::string_view>{}(std::string_view(key));
            }
            else
            {
                return std::hash<DecayedK>{}(key);
            }
        }

        template <typename K>
        static size_t make_seed(const K& key)
        {
            return hash_single(key);
        }

        template <typename K1, typename K2, typename... Keys>
        static size_t make_seed(const K1& k1, const K2& k2, const Keys&... keys)
        {
            size_t seed = hash_single(k1);
            auto mix = [&seed](const auto& key)
            {
                size_t h = hash_single(key);
                seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };
            mix(k2);
            (mix(keys), ...);
            return seed;
        }

        template <SampleableContainer Container>
        static size_t sample_index_modulo(const Container& container, size_t seed)
        {
            const size_t count = std::size(container);
            if (count == 0) return 0;
            return seed % count;
        }

        template <SampleableContainer Container>
        static size_t sample_index_rng(const Container& container, size_t seed)
        {
            const size_t count = std::size(container);
            if (count == 0) return 0;
            std::mt19937 rng(static_cast<unsigned int>(seed));
            std::uniform_int_distribution<uint64_t> dist(0, static_cast<uint64_t>(count - 1));
            return static_cast<size_t>(dist(rng));
        }

        template <SampleableContainer Container, typename... Keys>
            requires (sizeof...(Keys) > 0)
        static decltype(auto) sample_element(const Container& container, const Keys&... keys)
        {
            const size_t seed = make_seed(keys...);
            const size_t idx = sample_index_modulo(container, seed);
            return container[idx];
        }

        template <SampleableContainer Container>
        static decltype(auto) sample_element_rng(const Container& container, size_t seed)
        {
            const size_t idx = sample_index_rng(container, seed);
            return container[idx];
        }
    };
}
