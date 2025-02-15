#pragma once

#include <fstream>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <utility>

#include "hpfw/utils.h"

namespace hpfw::cache {

    template<typename Algo>
    class DriveCache {
    public:
        explicit DriveCache(std::string cache) : cache_dir(std::move(cache)) {
            if (!std::filesystem::exists(cache_dir)) {
                std::filesystem::create_directory(cache_dir);
                std::filesystem::create_directory(cache_dir + "spectros");
            }
        }

        ~DriveCache() = default;

        void set_spectro(const std::string &filename, const typename Algo::Spectrogram &f) const {
            const auto stem = std::string(std::filesystem::path(filename).stem());
            save(cache_dir + "spectros/" + stem, f);
        }

        void set_cov(const typename Algo::CovarianceMatrix &accum_cov) const {
            save(cache_dir + "accum_cov.cereal", accum_cov);
        }

        void set_filters(const typename Algo::Filters &f) const {
            save(cache_dir + "filters.cereal", f);
        }

        auto get_spectros() const {
            namespace fs = std::filesystem;

            filenames = utils::get_dir_files(cache_dir + "spectros/");

            return boost::make_iterator_range(filenames.cbegin(), filenames.cend()) |
                   boost::adaptors::transformed(load_spectro);
        }

        void get_cov(typename Algo::CovarianceMatrix &accum_cov) const {
            load(cache_dir + "accum_cov.cereal", accum_cov);
        }

        void get_filters(typename Algo::Filters &filters) const {
            load(cache_dir + "filters.cereal", filters);
        }

        auto size() const -> uint64_t {
            return utils::count_dir_files(cache_dir + "spectros/");
        }

    private:
        const std::string cache_dir;
        mutable std::vector<std::string> filenames;

        template<typename T>
        static void save(const std::string &filename, const T &obj) {
            std::ofstream os(filename, std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(obj);
        }

        template<typename T>
        static void load(const std::string &filename, T &obj) {
            if (!std::filesystem::exists(filename)) {
                return;
            }

            std::ifstream is(filename, std::ios::binary);
            cereal::BinaryInputArchive archive(is);
            archive(obj);
        }

        static auto load_spectro(const std::string &f) -> std::pair<std::string, typename Algo::Spectrogram> {
            typename Algo::Spectrogram spectro;
            load(f, spectro);
            return std::make_pair(f, std::move(spectro));
        }

    };

} // hpfw::cache
