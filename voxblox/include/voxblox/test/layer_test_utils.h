#ifndef VOXBLOX_TEST_LAYER_TEST_UTILS_H_
#define VOXBLOX_TEST_LAYER_TEST_UTILS_H_

#include <gtest/gtest.h>

#include "voxblox/core/layer.h"
#include "voxblox/core/voxel.h"

namespace voxblox {
namespace test {

template <typename VoxelType>
class LayerTest {
 public:
  void CompareLayers(const Layer<VoxelType>& layer_A,
                     const Layer<VoxelType>& layer_B) const {
    EXPECT_NEAR(layer_A.voxel_size(), layer_B.voxel_size(), kTolerance);
    EXPECT_NEAR(layer_A.block_size(), layer_B.block_size(), kTolerance);
    EXPECT_EQ(layer_A.voxels_per_side(), layer_B.voxels_per_side());
    EXPECT_EQ(layer_A.getNumberOfAllocatedBlocks(),
              layer_B.getNumberOfAllocatedBlocks());

    BlockIndexList blocks_A, blocks_B;
    layer_A.getAllAllocatedBlocks(&blocks_A);
    layer_B.getAllAllocatedBlocks(&blocks_B);
    EXPECT_EQ(blocks_A.size(), blocks_B.size());
    for (const BlockIndex& index_A : blocks_A) {
      BlockIndexList::const_iterator it =
          std::find(blocks_B.begin(), blocks_B.end(), index_A);
      if (it != blocks_B.end()) {
        const Block<VoxelType>& block_A = layer_A.getBlockByIndex(index_A);
        const Block<VoxelType>& block_B = layer_B.getBlockByIndex(*it);
        CompareBlocks(block_A, block_B);
      } else {
        ADD_FAILURE();
        LOG(ERROR) << "Block at index [" << index_A.transpose()
                   << "] in layer_A does not exists in layer_B";
      }
    }
    for (const BlockIndex& index_B : blocks_B) {
      BlockIndexList::const_iterator it =
          std::find(blocks_A.begin(), blocks_A.end(), index_B);
      if (it != blocks_A.end()) {
        const Block<VoxelType>& block_B = layer_A.getBlockByIndex(index_B);
        const Block<VoxelType>& block_A = layer_B.getBlockByIndex(*it);
        CompareBlocks(block_B, block_A);
      } else {
        ADD_FAILURE();
        LOG(ERROR) << "Block at index [" << index_B.transpose()
                   << "] in layer_B does not exists in layer_A";
      }
    }

    EXPECT_EQ(layer_A.getMemorySize(), layer_B.getMemorySize());
  }

  void CompareBlocks(const Block<VoxelType>& block_A,
                     const Block<VoxelType>& block_B) const {
    EXPECT_NEAR(block_A.voxel_size(), block_B.voxel_size(), kTolerance);
    EXPECT_NEAR(block_A.block_size(), block_B.block_size(), kTolerance);
    EXPECT_EQ(block_A.voxels_per_side(), block_B.voxels_per_side());

    EXPECT_NEAR(block_A.origin().x(), block_B.origin().x(), kTolerance);
    EXPECT_NEAR(block_A.origin().y(), block_B.origin().y(), kTolerance);
    EXPECT_NEAR(block_A.origin().z(), block_B.origin().z(), kTolerance);

    EXPECT_EQ(block_A.num_voxels(), block_B.num_voxels());
    for (size_t voxel_idx = 0u; voxel_idx < block_A.num_voxels(); ++voxel_idx) {
      CompareVoxel(block_A.getVoxelByLinearIndex(voxel_idx),
                   block_B.getVoxelByLinearIndex(voxel_idx));
    }
  }

  void CompareVoxel(const VoxelType& voxel_A, const VoxelType& voxel_B) const;

  static constexpr double kTolerance = 1e-10;
};

template <typename VoxelType>
void LayerTest<VoxelType>::CompareVoxel(const VoxelType& voxel_A,
                                        const VoxelType& voxel_B) const {
  LOG(FATAL) << "Not implemented for this voxel type!";
}

template <>
void LayerTest<EsdfVoxel>::CompareVoxel(const EsdfVoxel& voxel_A,
                                        const EsdfVoxel& voxel_B) const {
  constexpr double kTolerance = 1e-10;

  CHECK_NEAR(voxel_A.distance, voxel_B.distance, kTolerance);
  CHECK_EQ(voxel_A.observed, voxel_B.observed);
  CHECK_EQ(voxel_A.in_queue, voxel_B.in_queue);
  CHECK_EQ(voxel_A.fixed, voxel_B.fixed);

  // TODO(helenol): is this not serialized?
  // CHECK_EQ(voxel_A.parent.x(), voxel_B.parent.x());
  // CHECK_EQ(voxel_A.parent.y(), voxel_B.parent.y());
  // CHECK_EQ(voxel_A.parent.z(), voxel_B.parent.z());
}

template <>
void LayerTest<OccupancyVoxel>::CompareVoxel(
    const OccupancyVoxel& voxel_A, const OccupancyVoxel& voxel_B) const {
  constexpr double kTolerance = 1e-10;

  CHECK_NEAR(voxel_A.probability_log, voxel_B.probability_log, kTolerance);
  CHECK_EQ(voxel_A.observed, voxel_B.observed);
}

template <>
void LayerTest<TsdfVoxel>::CompareVoxel(const TsdfVoxel& voxel_A,
                                        const TsdfVoxel& voxel_B) const {
  CHECK_NEAR(voxel_A.distance, voxel_B.distance, kTolerance);
  CHECK_NEAR(voxel_A.weight, voxel_B.weight, kTolerance);
  CHECK_EQ(voxel_A.color.r, voxel_B.color.r);
  CHECK_EQ(voxel_A.color.g, voxel_B.color.g);
  CHECK_EQ(voxel_A.color.b, voxel_B.color.b);
  CHECK_EQ(voxel_A.color.a, voxel_B.color.a);
}

template <typename VoxelType>
void SetUpTestLayer(const size_t block_volume_diameter,
                    Layer<VoxelType>* layer) {
  CHECK(layer);
  LOG(FATAL) << "Not implemented for this voxel type!";
}

template <>
void SetUpTestLayer(const size_t block_volume_diameter,
                    Layer<TsdfVoxel>* layer) {
  CHECK(layer);

  int32_t half_index_range = block_volume_diameter / 2;

  for (int32_t x = -half_index_range; x <= half_index_range; ++x) {
    for (int32_t y = -half_index_range; y <= half_index_range; ++y) {
      for (int32_t z = -half_index_range; z <= half_index_range; ++z) {
        BlockIndex block_idx = {x, y, z};
        Block<TsdfVoxel>::Ptr block = layer->allocateBlockPtrByIndex(block_idx);
        TsdfVoxel& voxel = block->getVoxelByLinearIndex(
            (x * z + y) % layer->voxels_per_side());
        voxel.distance = x * y * 0.66 + z;
        voxel.weight = y * z * 0.33 + x;
        voxel.color.r = static_cast<uint8_t>(x % 255);
        voxel.color.g = static_cast<uint8_t>(y % 255);
        voxel.color.b = static_cast<uint8_t>(z % 255);
        voxel.color.a = static_cast<uint8_t>(x + y % 255);

        block->has_data() = true;
      }
    }
  }
  double size_in_MB = static_cast<double>(layer->getMemorySize()) * 1e-6;
  std::cout << std::endl
            << "Set up a test TSDF layer of size " << size_in_MB << " MB";
}

template <>
void SetUpTestLayer(const size_t block_volume_diameter,
                    Layer<EsdfVoxel>* layer) {
  CHECK(layer);

  int32_t half_index_range = block_volume_diameter / 2;

  for (int32_t x = -half_index_range; x <= half_index_range; ++x) {
    for (int32_t y = -half_index_range; y <= half_index_range; ++y) {
      for (int32_t z = -half_index_range; z <= half_index_range; ++z) {
        BlockIndex block_idx = {x, y, z};
        Block<EsdfVoxel>::Ptr block = layer->allocateBlockPtrByIndex(block_idx);
        EsdfVoxel& voxel = block->getVoxelByLinearIndex(
            (x * z + y) % layer->voxels_per_side());

        voxel.distance = x * y * 0.66 + z;
        voxel.parent.x() = x % 255;
        voxel.parent.y() = y % 255;
        voxel.parent.z() = z % 255;

        voxel.observed = true;
        voxel.in_queue = true;
        voxel.fixed = true;

        block->has_data() = true;
      }
    }
  }
  double size_in_MB = static_cast<double>(layer->getMemorySize()) * 1e-6;
  std::cout << std::endl
            << "Set up a test Esdf layer of size " << size_in_MB << " MB";
}

template <>
void SetUpTestLayer(const size_t block_volume_diameter,
                    Layer<OccupancyVoxel>* layer) {
  CHECK(layer);

  int32_t half_index_range = block_volume_diameter / 2;

  for (int32_t x = -half_index_range; x <= half_index_range; ++x) {
    for (int32_t y = -half_index_range; y <= half_index_range; ++y) {
      for (int32_t z = -half_index_range; z <= half_index_range; ++z) {
        BlockIndex block_idx = {x, y, z};
        Block<OccupancyVoxel>::Ptr block =
            layer->allocateBlockPtrByIndex(block_idx);
        OccupancyVoxel& voxel = block->getVoxelByLinearIndex(
            (x * z + y) % layer->voxels_per_side());

        voxel.probability_log = x * y * 0.66 + z;
        voxel.observed = true;

        block->has_data() = true;
      }
    }
  }
  double size_in_MB = static_cast<double>(layer->getMemorySize()) * 1e-6;
  std::cout << std::endl
            << "Set up a test occupancy layer of size " << size_in_MB << " MB";
}

template <typename VoxelType>
bool IsSameVoxel(const VoxelType& voxel_A, const VoxelType& voxel_B) {
  LOG(FATAL) << "Not implemented for this voxel type!";
  return false;
}

template <>
bool IsSameVoxel(const TsdfVoxel& voxel_A, const TsdfVoxel& voxel_B) {
  bool is_the_same = true;

  constexpr double kTolerance = 1e-10;

  is_the_same &= std::abs(voxel_A.distance - voxel_B.distance) < kTolerance;
  is_the_same &= std::abs(voxel_A.weight - voxel_B.weight) < kTolerance;
  is_the_same &= voxel_A.color.r == voxel_B.color.r;
  is_the_same &= voxel_A.color.g == voxel_B.color.g;
  is_the_same &= voxel_A.color.b == voxel_B.color.b;
  is_the_same &= voxel_A.color.a == voxel_B.color.a;

  return is_the_same;
}

template <>
bool IsSameVoxel(const EsdfVoxel& voxel_A, const EsdfVoxel& voxel_B) {
  bool is_the_same = true;

  constexpr double kTolerance = 1e-10;

  is_the_same &= std::abs(voxel_A.distance - voxel_B.distance) < kTolerance;
  is_the_same &= voxel_A.observed == voxel_B.observed;
  is_the_same &= voxel_A.in_queue == voxel_B.in_queue;
  is_the_same &= voxel_A.fixed == voxel_B.fixed;

  is_the_same &= voxel_A.parent.x() == voxel_B.parent.x();
  is_the_same &= voxel_A.parent.y() == voxel_B.parent.y();
  is_the_same &= voxel_A.parent.z() == voxel_B.parent.z();

  return is_the_same;
}

template <>
bool IsSameVoxel(const OccupancyVoxel& voxel_A, const OccupancyVoxel& voxel_B) {
  bool is_the_same = true;

  constexpr double kTolerance = 1e-10;

  is_the_same &=
      std::abs(voxel_A.probability_log - voxel_B.probability_log) < kTolerance;
  is_the_same &= voxel_A.observed == voxel_B.observed;

  return is_the_same;
}

template <typename VoxelType>
bool IsSameBlock(const Block<VoxelType>& block_A,
                 const Block<VoxelType>& block_B) {
  bool is_the_same = true;

  constexpr double kTolerance = 1e-10;

  is_the_same &=
      std::abs(block_A.voxel_size() - block_B.voxel_size()) < kTolerance;
  is_the_same &=
      std::abs(block_A.block_size() - block_B.block_size()) < kTolerance;
  is_the_same &= block_A.voxels_per_side() == block_B.voxels_per_side();

  is_the_same &=
      std::abs(block_A.origin().x() - block_B.origin().x()) < kTolerance;
  is_the_same &=
      std::abs(block_A.origin().y() - block_B.origin().y()) < kTolerance;
  is_the_same &=
      std::abs(block_A.origin().z() - block_B.origin().z()) < kTolerance;

  is_the_same &= block_A.num_voxels() == block_B.num_voxels();

  for (size_t voxel_idx = 0u; voxel_idx < block_A.num_voxels(); ++voxel_idx) {
    is_the_same &= IsSameVoxel(block_A.getVoxelByLinearIndex(voxel_idx),
                               block_B.getVoxelByLinearIndex(voxel_idx));
  }
  return is_the_same;
}

template <typename VoxelType>
bool IsSameLayer(const Layer<VoxelType>& layer_A,
                 const Layer<VoxelType>& layer_B) {
  constexpr double kTolerance = 1e-10;

  bool is_the_same = true;

  is_the_same &=
      std::abs(layer_A.voxel_size() - layer_B.voxel_size()) < kTolerance;
  is_the_same &=
      std::abs(layer_A.block_size() - layer_B.block_size()) < kTolerance;

  is_the_same &= layer_A.voxels_per_side() == layer_B.voxels_per_side();
  is_the_same &= layer_A.getNumberOfAllocatedBlocks() ==
                 layer_B.getNumberOfAllocatedBlocks();

  BlockIndexList blocks_A, blocks_B;
  layer_A.getAllAllocatedBlocks(&blocks_A);
  layer_B.getAllAllocatedBlocks(&blocks_B);
  is_the_same &= blocks_A.size() == blocks_B.size();

  for (const BlockIndex& index_A : blocks_A) {
    BlockIndexList::const_iterator it =
        std::find(blocks_B.begin(), blocks_B.end(), index_A);
    if (it != blocks_B.end()) {
      const Block<VoxelType>& block_A = layer_A.getBlockByIndex(index_A);
      const Block<VoxelType>& block_B = layer_B.getBlockByIndex(*it);
      bool is_same_block = IsSameBlock(block_A, block_B);
      LOG_IF(ERROR, !is_same_block)
          << "Block at index [" << index_A.transpose()
          << "] in layer_A is not the same as in layer_B";
      is_the_same &= is_same_block;
    } else {
      LOG(ERROR) << "Block at index [" << index_A.transpose()
                 << "] in layer_A does not exists in layer_B";
      return false;
    }
  }
  for (const BlockIndex& index_B : blocks_B) {
    BlockIndexList::const_iterator it =
        std::find(blocks_A.begin(), blocks_A.end(), index_B);
    if (it != blocks_A.end()) {
      const Block<VoxelType>& block_B = layer_A.getBlockByIndex(index_B);
      const Block<VoxelType>& block_A = layer_B.getBlockByIndex(*it);
      bool is_same_block = IsSameBlock(block_B, block_A);
      LOG_IF(ERROR, !is_same_block)
          << "Block at index [" << index_B.transpose()
          << "] in layer_B is not the same as in layer_A";
      is_the_same &= is_same_block;
    } else {
      LOG(ERROR) << "Block at index [" << index_B.transpose()
                 << "] in layer_B does not exists in layer_A";
      return false;
    }
  }

  is_the_same &= layer_A.getMemorySize() == layer_B.getMemorySize();
  return is_the_same;
}

}  // namespace test
}  // namespace voxblox

#endif  // VOXBLOX_TEST_LAYER_TEST_UTILS_H_
