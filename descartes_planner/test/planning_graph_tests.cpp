/*
 * Software License Agreement (Apache License)
 *
 * Copyright (c) 2015, Southwest Research Institute
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
 */

#include <descartes_planner/planning_graph.h>
#include <descartes_trajectory/joint_trajectory_pt.h>
#include <descartes_trajectory_test/cartesian_robot.h>
#include <boost/make_shared.hpp>

#include <gtest/gtest.h>

static boost::shared_ptr<descartes_core::RobotModel> makeTestRobot()
{
  const auto max_joint_vel = 1.0;
  const auto dof = 6;
  return boost::shared_ptr<descartes_core::RobotModel>(
    new descartes_trajectory_test::CartesianRobot(5.0, 0.001, std::vector<double>(dof, max_joint_vel))
  );
}

static std::vector<descartes_core::TrajectoryPtPtr> twoPoints()
{
  auto start_pt = boost::make_shared<descartes_trajectory::JointTrajectoryPt>(std::vector<double>(6, 0.0));
  auto stop_pt = boost::make_shared<descartes_trajectory::JointTrajectoryPt>(std::vector<double>(6, 1.0));

  std::vector<descartes_core::TrajectoryPtPtr> ret;
  ret.push_back(start_pt);
  ret.push_back(stop_pt);
  return ret;
}

static descartes_core::TrajectoryPtPtr thirdTestPoint()
{
  return boost::make_shared<descartes_trajectory::JointTrajectoryPt>(std::vector<double>(6, 2.0));
}

static std::vector<descartes_core::TrajectoryPtPtr> threePoints()
{
  auto pt = thirdTestPoint();
  auto vec = twoPoints();
  vec.push_back(pt);
  return vec;
} 

TEST(PlanningGraph, setup)
{
  // Create robot
  auto robot = makeTestRobot();
  // Create planner
  descartes_planner::PlanningGraph graph {robot};
  // Add a trajectory
  auto points = threePoints();
  ASSERT_TRUE(graph.insertGraph(points));

  double cost;
  std::list<descartes_trajectory::JointTrajectoryPt> out;
  EXPECT_TRUE(graph.getShortestPath(cost, out)); 
}

TEST(PlanningGraph, custom_cost_fn)
{
  // Create robot
  auto robot = makeTestRobot();
  auto points = threePoints();

  auto custom_cost_fn = [] (const double* a, const double* b) {
    double cost = 0.0;
    for (int i = 0; i < 6; ++i) cost += std::abs(a[i] - b[i]);
    return cost;
  };

  // Create planner
  descartes_planner::PlanningGraph graph {robot, custom_cost_fn};

  ASSERT_TRUE(graph.insertGraph(points));
  double cost;
  std::list<descartes_trajectory::JointTrajectoryPt> out;
  EXPECT_TRUE(graph.getShortestPath(cost, out));
}

TEST(PlanningGraph, insert_then_add_point)
{
  // Create robot
  auto robot = makeTestRobot();
  auto points = twoPoints();

  // Create planner
  descartes_planner::PlanningGraph graph {robot};

  // initialize graph
  ASSERT_TRUE(graph.insertGraph(points));

  auto new_pt = thirdTestPoint();

  ASSERT_TRUE(graph.addTrajectory( new_pt, points.back()->getID(), descartes_core::TrajectoryPt::ID::make_nil() ));
  
  double cost;
  std::list<descartes_trajectory::JointTrajectoryPt> out;
  ASSERT_TRUE(graph.getShortestPath(cost, out));

  EXPECT_TRUE(out.size() == 3);
  EXPECT_TRUE(cost != 0.0);
}