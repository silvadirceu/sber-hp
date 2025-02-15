#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

namespace hpfw {

    /// HashprintHandle provides "Collector" classes with the necessary tools to calculate hashprints.
    ///
    /// Algorithm (taken from Audio Hashprints: Theory & Application):
    ///
    /// 1. Compute spectrogram. The first step is to compute a time-frequency representation of audio.
    /// This time-frequency representation can be selected to suit the characteristics of the problem at hand.
    /// At the end of this first step, the audio is represented at each frame by a vector of dimension B,
    /// where B is the number of frequency subbands.
    ///
    /// 2. Collect context frames. In addition to looking at the audio frame of interest, we also look at the
    /// neighboring frames to its left and right. When computing the fingerprint at a particular frame, we consider
    /// w frames of context. At the end of this second step, we represent each frame with a vector of dimension Bw.
    ///
    /// 3. Apply spectro-temporal filters. At each frame we apply N different spectro-temporal filters in order
    /// to compute N different spectro-temporal features. Each spectro-temporal feature is a linear combination of
    /// the spectrogram energy values for the current frame and surrounding context frames. The weights of this
    /// linear combination are specified by the coefficients in the spectro-temporal filters. These filters are
    /// learned in an unsupervised manner by solving a sequence of optimization problems.
    /// At the end of this third step, we have N spectro-temporal features per frame.
    ///
    /// 4. Compute deltas. For each of our N features, we compute the change in the feature value over a time lag T.
    /// If the feature value at frame n is given by xn, then the corresponding delta feature will be ∆n = xn − xn+T .
    /// At the end of this fourth step, we have N spectro-temporal delta features per frame.
    ///
    /// 5. Apply threshold. Each of the N delta features is compared to a threshold value of 0, which results in a
    /// binary value. Each of these binary values thus represents whether the delta feature is increasing or
    /// decreasing over time (across the time lag T). At the end of the fifth step, we have N binary values per frame.
    ///
    /// 6. Bit packing. The N binary values are packed into a single 32-bit or 64-bit integer which represents the
    /// hashprint value for a single frame. This compact binary representation will allow us to store fingerprints
    /// in memory efficiently, do reverse indexing, or compute Hamming distance between hashprints very quickly.
    ///
    /// \tparam N - hashprint representation, integer
    /// \tparam SpectrogramHandler - provides spectrogram
    /// \tparam FramesContext - number of context frames used
    /// \tparam T - time lag
    template<typename N,
            typename SpectrogramHandler,
            size_t FramesContext,
            size_t T>
    class HashprintHandle {
    public:
        using Spectrogram = typename SpectrogramHandler::Spectrogram;
        /// Number of rows in spectrogram
        static constexpr size_t SpectroRows = Spectrogram::RowsAtCompileTime;
        /// Size of one frame
        static constexpr size_t FrameSize = SpectroRows * FramesContext;
        /// Number of context frames on one side (left or right)
        static constexpr size_t W = FramesContext / 2;
        /// Number of filters is equal to number of bits in hashprint representation
        static constexpr size_t NumOfFilters = sizeof(N) * 8;

        using Frames = Eigen::Matrix<float, FrameSize, Eigen::Dynamic, Eigen::RowMajor>;
        using CovarianceMatrix = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>; // to pass static_assert
        using Filters = Eigen::Matrix<float, NumOfFilters, Eigen::Dynamic>;
        using Fingerprint = Eigen::Matrix<bool, NumOfFilters, Eigen::Dynamic>;
        using Hashprint = std::vector<N>;

        const SpectrogramHandler sh;

        HashprintHandle() : sh() {}

        ~HashprintHandle() = default;

        /// Add context to frames.
        static auto calc_frames(const Spectrogram &spectro) -> Frames {
            using Eigen::Matrix;
            using Eigen::Dynamic;
            using Eigen::RowMajor;

            Frames frames(FrameSize, spectro.cols() - 2 * W + 1);
            for (size_t i = W, cnt = 0; i < spectro.cols() - W + 1; ++i, ++cnt) {
                Matrix<float, Dynamic, Dynamic, RowMajor> x = spectro.block(0, i - W, SpectroRows, FramesContext);
                x.resize(FrameSize, 1);

                frames.col(cnt) = std::move(x);
            }

            return frames;
        }

        /// Calculate covariance matrix.
        static auto calc_cov(const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat) -> CovarianceMatrix {
            using Eigen::Matrix;
            using Eigen::Dynamic;

            const Matrix<float, Dynamic, Dynamic> centered = mat.rowwise() - mat.colwise().mean();
            return (centered.adjoint() * centered) / float(mat.rows() - 1);
        }

        /// Find top N eigen vectors - these are the filters.
        static auto calc_filters(const CovarianceMatrix &cov) -> Filters {
            Eigen::SelfAdjointEigenSolver<CovarianceMatrix> solver(cov);
            return solver.eigenvectors()
                    .rowwise()
                    .reverse()
                    .transpose()
                    .block(0, 0, NumOfFilters, FrameSize);
        }

        /// Calculate deltas and apply threshold.
        static auto calc_fingerprint(const Eigen::Matrix<float, NumOfFilters, Eigen::Dynamic> &f) -> Fingerprint {
            using Eigen::Matrix;
            using Eigen::Dynamic;

            Fingerprint fp(NumOfFilters, f.cols() - T);
            for (size_t i = 0; i < f.cols() - T; ++i) {
                fp.col(i) = (f.col(i) - f.col(i + T)).array() >= 0;
            }

            return fp;
        }

        static auto fingerprint_to_hashprint(const Fingerprint &fp) -> Hashprint {
            Hashprint hp(static_cast<unsigned long>(fp.cols()));
            for (size_t i = 0; i < fp.cols(); ++i) {
                hp[i] = bool_col_to_num(fp.col(i));
            }
            return hp;
        }

    private:
        /// Pack bool column into integer.
        static auto bool_col_to_num(const Eigen::Matrix<bool, NumOfFilters, 1> &c) -> N {
            size_t p = 0;
            return c.reverse().unaryExpr([&p](bool x) -> N {
                return x * pow(2, p++);
            }).sum();
        }

    }; // HashprintHandle

} // hpfw
