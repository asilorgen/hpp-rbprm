#
# Copyright (c) 2010, 2011 CNRS
# Authors: Florent Lamiraux
#
#
# This file is part of hpp-core
# hpp-core is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
#
# hpp-core is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Lesser Public License for more details.  You should have
# received a copy of the GNU Lesser General Public License along with
# hpp-core  If not, see
# <http://www.gnu.org/licenses/>.

SET(LIBRARY_NAME ${PROJECT_NAME})

SET(${LIBRARY_NAME}_SOURCES
	rbprm-shooter.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-shooter.hh
	rbprm-validation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-validation.hh
  rbprm-path-validation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-path-validation.hh
        rbprm-rom-validation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-rom-validation.hh
	rbprm-device.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-device.hh
	rbprm-limb.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-limb.hh
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-dependant.hh
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/interpolation-constraints.hh
        interpolation/interpolation-constraints.cc
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-constraint-helper.hh
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-constraint-helper.inl
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-constraint-utils.hh
        interpolation/effector-rrt.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/spline/effector-rrt.hh
        interpolation/bezier-path.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/spline/bezier-path.hh
        interpolation/rbprm-path-interpolation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/rbprm-path-interpolation.hh
        interpolation/time-constraint-shooter.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-constraint-shooter.hh
        interpolation/limb-rrt-shooter.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/limb-rrt-shooter.hh
        interpolation/com-rrt-shooter.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/com-rrt-shooter.hh
				projection/projection.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/projection/projection.hh
				contact_generation/contact_generation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/contact_generation/contact_generation.hh
				contact_generation/kinematics_constraints.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/contact_generation/kinematics_constraints.hh
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/reports.hh
				contact_generation/algorithm.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/contact_generation/algorithm.hh
				contact_generation/reachability.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/contact_generation/reachability.hh
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/limb-rrt.hh
        interpolation/limb-rrt.cc
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/com-rrt.hh
        interpolation/com-rrt.cc
        interpolation/time-constraint-path-validation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-constraint-path-validation.hh
        ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-constraint-steering.hh
        interpolation/time-constraint-path.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/time-constraint-path.hh
        interpolation/com-trajectory.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/com-trajectory.hh
        interpolation/polynom-trajectory.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/interpolation/polynom-trajectory.hh
        rbprm-fullbody.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-fullbody.hh
        rbprm-state.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-state.hh
        sampling/sample.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/sampling/sample.hh
        sampling/analysis.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/sampling/analysis.hh
        sampling/heuristic-tools.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/sampling/heuristic-tools.hh
        sampling/heuristic.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/sampling/heuristic.hh
        sampling/sample-db.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/sampling/sample-db.hh
        tools.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/tools.hh
        stability/stability.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/stability/stability.hh
        stability/support.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/stability/support.hh
        ik-solver.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/ik-solver.hh
        utils/stop-watch.cc ${PROJECT_SOURCE_DIR}/include/utils/stop-watch.hh
        utils/algorithms.cc ${PROJECT_SOURCE_DIR}/include/utils/algorithms.h
        rbprm-profiler.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/rbprm-profiler.hh
#        planner/parabola-planner.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/parabola-planner.hh
        planner/steering-method-parabola.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/steering-method-parabola.hh
        planner/parabola-path.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/parabola-path.hh
        planner/timed-parabola-path.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/timed-parabola-path.hh
        planner/dynamic-planner.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/dynamic-planner.hh
        planner/rbprm-steering-kinodynamic.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/rbprm-steering-kinodynamic.hh
        planner/rbprm-node.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/rbprm-node.hh
        dynamic/dynamic-validation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/dynamic/dynamic-validation.hh
        dynamic/dynamic-path-validation.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/dynamic/dynamic-path-validation.hh
        planner/random-shortcut-dynamic.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/random-shortcut-dynamic.hh
        planner/oriented-path-optimizer.cc ${PROJECT_SOURCE_DIR}/include/hpp/rbprm/planner/oriented-path-optimizer.hh


  )

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
ADD_LIBRARY(${LIBRARY_NAME}
	SHARED
  ${${LIBRARY_NAME}_SOURCES}
  )

TARGET_LINK_LIBRARIES(${LIBRARY_NAME} centroidal-dynamics-lib bezier-com-traj)

PKG_CONFIG_USE_DEPENDENCY(${LIBRARY_NAME} hpp-core)
PKG_CONFIG_USE_DEPENDENCY(${LIBRARY_NAME} hpp-util)
PKG_CONFIG_USE_DEPENDENCY(${LIBRARY_NAME} hpp-statistics)
PKG_CONFIG_USE_DEPENDENCY(${LIBRARY_NAME} hpp-constraints)
PKG_CONFIG_USE_DEPENDENCY(${LIBRARY_NAME} hpp-fcl)
PKG_CONFIG_USE_DEPENDENCY(${LIBRARY_NAME} hpp-model)
PKG_CONFIG_USE_DEPENDENCY(${LIBRARY_NAME} hpp-model-urdf)

INSTALL(TARGETS ${LIBRARY_NAME} DESTINATION lib)
