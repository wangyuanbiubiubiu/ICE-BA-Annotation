/******************************************************************************
 * Copyright 2017 Baidu Robotic Vision Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#ifndef XP_INCLUDE_XP_HELPER_PARAM_H_
#define XP_INCLUDE_XP_HELPER_PARAM_H_

#include <yaml-cpp/yaml.h>

#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <limits>
namespace XP {

class ParamBase {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

 public:
  ParamBase() {}
  virtual bool LoadFromYaml(const std::string& filename) = 0;
  virtual bool WriteToYaml(const std::string& filename) = 0;
  virtual bool LoadFromCvYaml(const std::string& filename) = 0;
  virtual bool WriteToCvYaml(const std::string& filename) = 0;

 protected:
  void serialize(const std::string& filename, const YAML::Emitter& emitter);
  YAML::Node deserialize(const std::string& filename);

  YAML::Node base_node_;
};

class AlgorithmParam : public ParamBase {
 public:
  AlgorithmParam() {
  }

  bool LoadFromYaml(const std::string& filename) override;
  bool WriteToYaml(const std::string& filename) override;
  bool LoadFromCvYaml(const std::string& filename) override;
  bool WriteToCvYaml(const std::string& filename) override;
  struct FeatDetParam_t {
    int request_feat_num = 70;
    int pyra_level = 2;
    int fast_det_thresh = 10;
    int uniform_radius = 40;
    float min_feature_distance_over_baseline_ratio = 3;
    float max_feature_distance_over_baseline_ratio = 3000;
    int feature_track_length_thresh = 25;
    float feature_track_dropout_rate = 0.3f;
  } FeatDetParam;
  struct Tracking_t {
    float max_feature_search_range = 10.f / 400.f;
    float orb_match_dist_thresh = 0.3 * 255;
    float orb_match_thresh_test_ratio = 0.9;
    float feature_uncertainty = 5;
    int imaging_FPS = 20;
    int imaging_exposure = 100;
    int imaging_gain = 100;
    int aec_index = 100;
    bool use_of_id = true;
    bool use_april_tag = false;
    std::string slave_det = "direct";
    bool undistort_before_vio = true;
  } Tracking;
};

//内外参
class DuoCalibParam : public ParamBase {
 public:
  DuoCalibParam();
  ~DuoCalibParam() {}

  bool LoadFromYaml(const std::string& filename) override;
  bool WriteToYaml(const std::string& filename) override;
  bool LoadFromCvYaml(const std::string& filename) override;
  bool WriteToCvYaml(const std::string& filename) override;
  bool LoadCamCalibFromYaml(const std::string& filename);
  bool LoadImuCalibFromYaml(const std::string& filename);

  // init undistort map from camera K and distort
  bool initUndistortMap(const cv::Size& new_img_size);

  struct Imu_t {
    Eigen::Matrix3f accel_TK;
    Eigen::Vector3f accel_bias;//加速度偏置
    Eigen::Matrix3f gyro_TK;
    Eigen::Vector3f gyro_bias;//陀螺仪偏置
    Eigen::Vector3f accel_noise_var;  // m / sec^2
    Eigen::Vector3f angv_noise_var;  // rad / sec
    Eigen::Matrix4f D_T_I;//imu到设备系的变换
    Eigen::Matrix4f undist_D_T_I;//立体矫正后imu到设备坐标系下的变换
  } Imu;

  //双目相机的标定参数
  struct Camera_t {
      //左右相机到设备坐标系的变换,即Tdc0,Tdc1,设备坐标系固连于左相机坐标系
    std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>> D_T_C_lr;
    std::vector<Eigen::Matrix3f, Eigen::aligned_allocator<Eigen::Matrix3f>> cameraK_lr;//eigen形式的左右相机内参
    std::vector<cv::Matx33f> cv_camK_lr;//cv形式的左右相机内参
    std::vector<cv::Mat_<float>> cv_dist_coeff_lr;//左右相机的畸变参数
    // the boundary of images in uv coordinate
    //分别是左右相机的边界,顺序是左x上y右x下y
    std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> lurd_lr;
    //双目立体矫正去畸变后的x,y的映射
    std::vector<cv::Mat> undistort_map_op1_lr;
    std::vector<cv::Mat> undistort_map_op2_lr;
    std::vector<cv::Matx33f> cv_undist_K_lr;//双目矫正后的新的内参
    std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>> undist_D_T_C_lr;//立体矫正后的双目到设备坐标系的变换,一样是设备坐标系固连在左相机上
    cv::Matx44f Q;  // //视差深度映射矩阵4x4 convert disparity to depth. See cv::reprojectImageTo3D
    cv::Size img_size;//图像大小
    bool fishEye = false; //是否是鱼眼相机 by wya
  } Camera;

  Eigen::Matrix3f C_R_B;
  Eigen::Vector3f C_p_B;
  std::string device_id;
  enum SensorType {
    UNKNOWN = 0,
    LI = 2,
    XP = 3,
    XP2 = 4,
    XP3 = 5,
  } sensor_type;
};

// Helper functions to load calibration parameters from calib.yaml
bool get_calib_file_from_device_id(const std::string& device_id,
                                   std::string* calib_file_ptr);
bool load_imu_calib_param(const std::string& device_id,
                          DuoCalibParam* duo_calib_param_ptr);

bool load_camera_calib_param(const std::string& calib_file,
                             XP::DuoCalibParam* duo_calib_param_ptr);
bool save_camera_calib_param(const std::string& calib_file,
                             const XP::DuoCalibParam& duo_calib_param);
}  // namespace XP
#endif  // XP_INCLUDE_XP_HELPER_PARAM_H_
