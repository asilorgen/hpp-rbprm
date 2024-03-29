//
// Copyright (c) 2017 CNRS
// Authors: Steve Tonneau
//
// This file is part of hpp-rbprm
// hpp-core is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-core is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-core  If not, see
// <http://www.gnu.org/licenses/>.

#include <hpp/rbprm/projection/projection.hh>
#include <hpp/rbprm/interpolation/interpolation-constraints.hh>
#include <hpp/model/joint.hh>
#include <hpp/constraints/relative-com.hh>
#include <hpp/constraints/symbolic-calculus.hh>
#include <hpp/constraints/symbolic-function.hh>

#ifdef PROFILE
    #include "hpp/rbprm/rbprm-profiler.hh"
#endif




namespace hpp {
namespace rbprm {
namespace projection{

const double epsilon = 10e-3;

ProjectionReport::ProjectionReport(const ProjectionReport& from)
    : success_(from.success_)
    , result_(from.result_)
    , status_(from.status_)
{
    // NOTHING
}


std::vector<bool> setMaintainRotationConstraints()
{
    std::vector<bool> res;
    for(std::size_t i =0; i <3; ++i)
        res.push_back(true);
    return res;
}

void CreateContactConstraints(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const hpp::rbprm::State& currentState, core::ConfigProjectorPtr_t proj)
{
    model::DevicePtr_t device = fullBody->device_;
    std::vector<bool> cosntraintsR = setMaintainRotationConstraints();
    std::vector<std::string> fixed = currentState.fixedContacts(currentState);
    for(std::vector<std::string>::const_iterator cit = fixed.begin(); cit != fixed.end(); ++cit)
    {
        const std::string& effector = *cit;
        RbPrmLimbPtr_t limb = fullBody->GetLimbs().at(effector);
        const fcl::Vec3f& ppos  = currentState.contactPositions_.at(effector);
        JointPtr_t effectorJoint = device->getJointByName(limb->effector_->name());
        proj->add(core::NumericalConstraint::create (
                                constraints::deprecated::Position::create("",device,
                                                              effectorJoint,fcl::Vec3f(0,0,0), ppos)));
        if(limb->contactType_ == hpp::rbprm::_6_DOF)
        {
            const fcl::Matrix3f& rotation = currentState.contactRotation_.at(effector);
            proj->add(core::NumericalConstraint::create (constraints::deprecated::Orientation::create("", device,
                                                                              effectorJoint,
                                                                              rotation,
                                                                              cosntraintsR)));
        }
    }
}

void CreateRootPosConstraint(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const fcl::Vec3f& target, core::ConfigProjectorPtr_t proj)
{
    proj->add(core::NumericalConstraint::create (
                            constraints::deprecated::Position::create("",fullBody->device_,
                                                          fullBody->device_->rootJoint(),fcl::Vec3f(0,0,0), target)));
}

typedef constraints::PointCom PointCom;
typedef constraints::CalculusBaseAbstract<PointCom::ValueType_t, PointCom::JacobianType_t> s_t;
typedef constraints::SymbolicFunction<s_t> PointComFunction;
typedef constraints::SymbolicFunction<s_t>::Ptr_t PointComFunctionPtr_t;

void CreateComPosConstraint(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const fcl::Vec3f& target, core::ConfigProjectorPtr_t proj)
{
    model::DevicePtr_t device = fullBody->device_;
    core::ComparisonTypePtr_t equals = core::Equality::create ();
    model::CenterOfMassComputationPtr_t comComp = model::CenterOfMassComputation::create (device);
    comComp->add (device->rootJoint());
    comComp->computeMass ();
    PointComFunctionPtr_t comFunc = PointComFunction::create ("COM-walkgen",
        device, PointCom::create (comComp));
    NumericalConstraintPtr_t comEq = NumericalConstraint::create (comFunc, equals);
    comEq->nonConstRightHandSide() = target;
    proj->add(comEq);
    proj->updateRightHandSide();
}

void CreatePosturalTaskConstraint(hpp::rbprm::RbPrmFullBodyPtr_t fullBody,core::ConfigProjectorPtr_t proj){
  core::ComparisonTypePtr_t equals = core::Equality::create ();
  //hppDout(notice,"create postural task, in projection.cc, ref config = "<<model::displayConfig(fullBody->referenceConfig()));
  std::vector <bool> mask (fullBody->device_->numberDof(),false);
  Configuration_t weight(fullBody->device_->numberDof());

  // mask : 0 for the freeflyer and the extraDoFs :
  for(size_t i = 0 ; i < 3 ; i++){
    mask[i]=false;
  }
  for(size_t i = fullBody->device_->numberDof()-7 ; i < fullBody->device_->numberDof() ; i++){
    mask[i]=false;
  }




  // create a weight vector
  for(size_t i = 0 ; i < fullBody->device_->numberDof() ; ++i){
    weight[i] = 1.;
  }
  // TODO : retrieve it from somewhere, store it in fullbody ?
  // value here for hrp2, from Justin
  // : chest
  weight[6]= 10.;
  //head :
  for(size_t i = 7 ; i <= 9 ; i++)
    weight[i] = 1.;

  // root's orientation
  for(size_t i = 3 ; i < 6 ; i++){
    weight[i] = 50.;
  }

  for(size_t i = 3 ; i <= 9 ; i++){
    mask[i] = true;
  }
 // mask[5] = false; // z root rotation ????


  // normalize weight array :
  double moy =0;
  int num_active = 0;
  for (size_t i = 0 ; i < weight.size() ; i++){
    if(mask[i]){
      moy += weight[i];
      num_active++;
    }
  }
  moy = moy/num_active;
  for (size_t i = 0 ; i < weight.size() ; i++)
    weight[i] = weight[i]/moy;


  std::ostringstream oss;
  for (size_type i=0; i < mask.size (); ++i){
    oss << mask [i] << ",";
  }
  //hppDout(notice,"mask = "<<oss.str());

  constraints::ConfigurationConstraintPtr_t postFunc = constraints::ConfigurationConstraint::create("Postural_Task",fullBody->device_,fullBody->referenceConfig(),weight,mask);
  const NumericalConstraintPtr_t posturalTask = NumericalConstraint::create (postFunc, equals);
  proj->add(posturalTask,SizeIntervals_t (0),1);
  proj->updateRightHandSide();
}

ProjectionReport projectToRootPosition(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const fcl::Vec3f& target,
                                           const hpp::rbprm::State& currentState)
{
    ProjectionReport res;
    core::ConfigProjectorPtr_t proj = core::ConfigProjector::create(fullBody->device_,"proj", 1e-4, 40);
    CreateContactConstraints(fullBody, currentState, proj);
    CreateRootPosConstraint(fullBody, target, proj);
    model::Configuration_t configuration = currentState.configuration_;
    res.success_ = proj->apply(configuration);
    res.result_ = currentState;
    res.result_.configuration_ = configuration;
    return res;
}

typedef std::vector<model::JointPtr_t> T_Joint;

T_Joint getJointsFromLimbs(const rbprm::T_Limb limbs)
{
    T_Joint res;
    for(rbprm::CIT_Limb cit = limbs.begin(); cit != limbs.end(); ++cit)
        res.push_back(cit->second->limb_);
    return res;
}

bool not_a_limb(model::JointPtr_t cJoint, T_Joint limbs)
{
    for(T_Joint::const_iterator cit = limbs.begin(); cit != limbs.end(); ++cit)
    {
        if(cJoint->name() == (*cit)->name()) return false;
    }
    return true;
}

void LockFromRootRec(model::JointPtr_t cJoint, const std::vector<model::JointPtr_t>& jointLimbs, model::ConfigurationIn_t targetRootConfiguration, core::ConfigProjectorPtr_t& projector)
{
    if(not_a_limb(cJoint, jointLimbs))
    {
        core::size_type rankInConfiguration = (cJoint->rankInConfiguration ());
        projector->add(core::LockedJoint::create(cJoint,targetRootConfiguration.segment(rankInConfiguration, cJoint->configSize())));
        //if (cJoint->numberChildJoints() !=1)
        //    return;
        for(int i =0; i< cJoint->numberChildJoints(); ++i)
            LockFromRootRec(cJoint->childJoint(i), jointLimbs, targetRootConfiguration, projector);
    }
}

void LockFromRoot(hpp::model::DevicePtr_t device, const rbprm::T_Limb& limbs, model::ConfigurationIn_t targetRootConfiguration, core::ConfigProjectorPtr_t& projector)
{
    std::vector<model::JointPtr_t> jointLimbs = getJointsFromLimbs(limbs);
    model::JointPtr_t cJoint = device->rootJoint();
    LockFromRootRec(cJoint, jointLimbs, targetRootConfiguration, projector);
}


ProjectionReport projectToRootConfiguration(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const model::ConfigurationIn_t conf,
                                           const hpp::rbprm::State& currentState)
{
    ProjectionReport res;
    core::ConfigProjectorPtr_t proj = core::ConfigProjector::create(fullBody->device_,"proj", 1e-4, 40);
    CreateContactConstraints(fullBody, currentState, proj);
    LockFromRoot(fullBody->device_, fullBody->GetLimbs(), conf, proj);
    model::Configuration_t configuration = currentState.configuration_;
    res.success_ = proj->apply(configuration);
    res.result_ = currentState;
    res.result_.configuration_ = configuration;
    return res;
}

ProjectionReport setCollisionFree(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const core::CollisionValidationPtr_t& validation ,
                                  const std::string& limbName,const hpp::rbprm::State& currentState,bool sort)
{
    ProjectionReport res;
    res.result_ = currentState;
    model::Configuration_t configuration = currentState.configuration_;
    hpp::core::ValidationReportPtr_t valRep (new hpp::core::CollisionValidationReport);
    if(validation->validate(configuration, valRep) )
    {
        res.result_.configuration_ = configuration;
        res.success_ = true;
        //hppDout(notice,"Found collision free conf : current configuration was already valid !");
        return res;
    }

    RbPrmLimbPtr_t limb = fullBody->GetLimb(limbName);
    sampling::T_Sample samples(limb->sampleContainer_.samples_);
    if(sort){
        sampling::sample_greater sortAlgo;
        std::sort(samples.begin(),samples.end(),sortAlgo);
    }
    for(sampling::SampleVector_t::const_iterator cit = samples.begin();cit != samples.end(); ++cit)
    {
        sampling::Load(*cit, configuration);
        //hppDout(notice,"Set collision free : static value = "<<cit->staticValue_);
        if(validation->validate(configuration, valRep) )
        {
            res.result_.configuration_ = configuration;
            res.success_ = true;
            //hppDout(notice,"Found collision free conf !");
            return res;
        }
    }
    return res;
}

std::vector<bool> setRotationConstraints()
{
    std::vector<bool> res;
    for(std::size_t i =0; i <3; ++i)
    {
        res.push_back(true);
    }
    return res;
}

std::vector<bool> setTranslationConstraints()
{
    std::vector<bool> res;
    for(std::size_t i =0; i <3; ++i)
    {
        res.push_back(true);
    }
    return res;
}
ProjectionReport projectEffector(hpp::core::ConfigProjectorPtr_t proj, const hpp::rbprm::RbPrmFullBodyPtr_t& body, const std::string& limbId, const hpp::rbprm::RbPrmLimbPtr_t& limb,
                          core::CollisionValidationPtr_t validation, model::ConfigurationOut_t configuration,
                          const fcl::Matrix3f& rotationTarget, const std::vector<bool> &rotationFilter, const fcl::Vec3f& positionTarget, const fcl::Vec3f& normal,
                          const hpp::rbprm::State& current)
{
    //hppDout(notice,"Project effector : ");
    ProjectionReport rep;
    // Add constraints to resolve Ik
    rep.success_ = false;
    rep.result_ = current;
    // Add constraints to resolve Ik
    fcl::Transform3f localFrame, globalFrame;
    globalFrame.setTranslation(positionTarget);
    proj->add(core::NumericalConstraint::create (constraints::Position::create("",body->device_,
                                                                               limb->effector_,
                                                                               localFrame,
                                                                               globalFrame,
                                                                               setTranslationConstraints())));

    if(limb->contactType_ == hpp::rbprm::_6_DOF)
    {
        proj->add(core::NumericalConstraint::create (constraints::Orientation::create("",body->device_,
                                                                                      limb->effector_,
                                                                                      fcl::Transform3f(rotationTarget),
                                                                                      rotationFilter)));
    }
#ifdef PROFILE
    RbPrmProfiler& watch = getRbPrmProfiler();
    watch.start("ik");
#endif
    if(proj->apply(configuration))
    {
        //hppDout(notice,"apply contact constraints");
#ifdef PROFILE
        watch.stop("ik");
#endif
#ifdef PROFILE
        RbPrmProfiler& watch = getRbPrmProfiler();
        watch.start("collision");
#endif
        hpp::core::ValidationReportPtr_t valRep (new hpp::core::CollisionValidationReport);
        if(validation->validate(configuration, valRep))
    {
        //hppDout(notice,"No collision !");
#ifdef PROFILE
        watch.stop("collision");
#endif
        //hppDout(notice,"Projection successfull, add new contact info :");
        body->device_->currentConfiguration(configuration);
        body->device_->computeForwardKinematics();
        State tmp (current);
        tmp.contacts_[limbId] = true;
        tmp.contactPositions_[limbId] = limb->effector_->currentTransformation().getTranslation();
        tmp.contactRotation_[limbId] = limb->effector_->currentTransformation().getRotation();
        tmp.contactNormals_[limbId] = normal;
        tmp.contactOrder_.push(limbId);
        tmp.configuration_ = configuration;
        ++tmp.nbContacts;
        rep.success_ = true;
        rep.result_ = tmp;
        }else
        {
            #ifdef PROFILE
            watch.stop("collision");
            #endif
            //hppDout(notice,"state in collison after projection, report : "<<*valRep);
            //hppDout(notice,"state in collision : r(["<<model::displayConfig(configuration)<<"])");
        }
    }else{
        #ifdef PROFILE
        watch.stop("ik");
        #endif
        //hppDout(notice,"unable to apply contact constraints");

    }
    return rep;
}

fcl::Transform3f computeProjectionMatrix(const hpp::rbprm::RbPrmFullBodyPtr_t& body, const hpp::rbprm::RbPrmLimbPtr_t& limb, const model::ConfigurationIn_t configuration,
                                         const fcl::Vec3f& normal, const fcl::Vec3f& position)
{
    body->device_->currentConfiguration(configuration);
    body->device_->computeForwardKinematics();
    // the normal is given by the normal of the contacted object
    const fcl::Vec3f z = limb->effector_->currentTransformation().getRotation() * limb->normal_;
    const fcl::Matrix3f alignRotation = tools::GetRotationMatrix(z,normal);
    //hppDout(notice,"alignRotation : \n"<<alignRotation);
    const fcl::Matrix3f rotation = alignRotation * limb->effector_->currentTransformation().getRotation();
    //hppDout(notice,"rotation : \n"<<rotation);
    fcl::Vec3f posOffset = position - rotation * limb->offset_;
    posOffset = posOffset + normal * epsilon;
    return fcl::Transform3f(rotation,posOffset);
}

ProjectionReport projectToObstacle(core::ConfigProjectorPtr_t proj, const hpp::rbprm::RbPrmFullBodyPtr_t& body,const std::string& limbId, const hpp::rbprm::RbPrmLimbPtr_t& limb,
                                   core::CollisionValidationPtr_t validation, model::ConfigurationOut_t configuration, const hpp::rbprm::State& current,
                                   const fcl::Vec3f& normal, const fcl::Vec3f& position)
{
    fcl::Transform3f pM = computeProjectionMatrix(body, limb, configuration, normal, position);
    return projectEffector(proj, body, limbId, limb, validation, configuration, pM.getRotation(), setRotationConstraints(),pM.getTranslation(), normal, current);
}

// are p1 and p2 on the same side of the line AB ?
bool SameSide(const fcl::Vec3f& p1, const fcl::Vec3f& p2, const fcl::Vec3f& a, const fcl::Vec3f& b)
{
    fcl::Vec3f cp1 = (b-a).cross(p1-a);
    fcl::Vec3f cp2 = (b-a).cross(p2-a);
    return cp1.dot(cp2)>=0;
}

// is p inside ABC ?
bool PointInTriangle(const fcl::Vec3f& p, const fcl::Vec3f& a, const fcl::Vec3f& b, const fcl::Vec3f& c)
{
    return SameSide(p,a, b,c) && SameSide(p,b, a,c) && SameSide(p,c, a,b);
}


ProjectionReport projectSampleToObstacle(const hpp::rbprm::RbPrmFullBodyPtr_t& body,const std::string& limbId, const hpp::rbprm::RbPrmLimbPtr_t& limb,
                                         const sampling::OctreeReport& report, core::CollisionValidationPtr_t validation,
                                         model::ConfigurationOut_t configuration, const hpp::rbprm::State& current)
{
    sampling::Load(*report.sample_, configuration);
    const fcl::Vec3f& normal = report.normal_;
   // //hppDout(notice,"contact normal = "<<normal);
    fcl::Transform3f rootT = body->GetLimb(limbId)->limb_->parentJoint()->currentTransformation();
    //compute the orthogonal projection of the end effector on the plan :
    const fcl::Vec3f& pEndEff = (rootT.transform(report.sample_->effectorPosition_)); // compute absolute position (in world frame)
    fcl::Vec3f pos = pEndEff-(normal.dot(pEndEff-report.contact_.pos))*normal; // orthogonal projection on the obstacle surface
    // if position not in triangle take collision point as target
    if (!PointInTriangle(pos,report.v1_, report.v2_, report.v3_))
        pos = report.contact_.pos;
    ////hppDout(notice,"Effector position : "<<report.sample_->effectorPosition_);
    ////hppDout(notice,"pEndEff = ["<<pEndEff[0]<<","<<pEndEff[1]<<","<<pEndEff[2]<<"]");
    ////hppDout(notice,"pos = ["<<pos[0]<<","<<pos[1]<<","<<pos[2]<<"]");
    core::ConfigProjectorPtr_t proj = core::ConfigProjector::create(body->device_,"proj", 1e-4, 20);
    hpp::tools::LockJointRec(limb->limb_->name(), body->device_->rootJoint(), proj);
    return projectToObstacle(proj, body, limbId, limb, validation, configuration, current, normal, pos);
}

ProjectionReport projectStateToObstacle(const hpp::rbprm::RbPrmFullBodyPtr_t& body, const std::string& limbId, const hpp::rbprm::RbPrmLimbPtr_t& limb,
                                        const hpp::rbprm::State& current, const fcl::Vec3f &normal, const fcl::Vec3f &position)
{
   // core::CollisionValidationPtr_t dummy = core::CollisionValidation::create(body->device_);
    return projectStateToObstacle(body, limbId, limb, current, normal, position, body->GetCollisionValidation());
}

ProjectionReport projectStateToObstacle(const hpp::rbprm::RbPrmFullBodyPtr_t& body, const std::string& limbId, const hpp::rbprm::RbPrmLimbPtr_t& limb,
                                        const hpp::rbprm::State& current, const fcl::Vec3f &normal, const fcl::Vec3f &position, core::CollisionValidationPtr_t validation)
{
    hpp::rbprm::State state = current;
    state.RemoveContact(limbId);
    model::Configuration_t configuration = current.configuration_;
    core::ConfigProjectorPtr_t proj = core::ConfigProjector::create(body->device_,"proj", 1e-4, 1000);
    interpolation::addContactConstraints(body, body->device_,proj, state, state.fixedContacts(state));
    // get current normal orientation
    return projectToObstacle(proj, body, limbId, limb, validation, configuration, state, normal, position);
}


ProjectionReport projectToComPosition(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const fcl::Vec3f& target,
                                           const hpp::rbprm::State& currentState)
{
    ProjectionReport res;
    core::ConfigProjectorPtr_t proj = core::ConfigProjector::create(fullBody->device_,"proj", 1e-4, 1000);
    CreateContactConstraints(fullBody, currentState, proj);
    CreateComPosConstraint(fullBody, target, proj);
   /* CreatePosturalTaskConstraint(fullBody,proj);
    proj->lastIsOptional(true);
    proj->numOptimize(500);
    proj->lastAsCost(false);
    proj->errorThreshold(1e-3);*/

    model::Configuration_t configuration = currentState.configuration_;
    res.success_ = proj->apply(configuration);
    res.result_ = currentState;
    res.result_.configuration_ = configuration;
    return res;
}

std::vector<std::string> extractEffectorsName(const rbprm::T_Limb& limbs)
{
    std::vector<std::string> res;
    for(rbprm::T_Limb::const_iterator cit = limbs.begin(); cit != limbs.end(); ++cit)
        res.push_back(cit->first);
    return res;
}

ProjectionReport projectToColFreeComPosition(hpp::rbprm::RbPrmFullBodyPtr_t fullBody, const fcl::Vec3f& target,
                                           const hpp::rbprm::State& currentState)
{
    ProjectionReport res, tmp;
    core::ConfigProjectorPtr_t proj = core::ConfigProjector::create(fullBody->device_,"proj", 1e-3, 1000);
    CreateContactConstraints(fullBody, currentState, proj);
    CreateComPosConstraint(fullBody, target, proj);
    CreatePosturalTaskConstraint(fullBody,proj);
    proj->lastIsOptional(true);
    proj->numOptimize(500);
    proj->lastAsCost(true);
    proj->errorThreshold(1e-2);

    model::Configuration_t configuration = currentState.configuration_;
    res.success_ = proj->apply(configuration);
    res.result_ = currentState;
    res.result_.configuration_ = configuration;
    //hppDout(notice,"Project to col free, first projection done : "<<res.success_);
    //hppDout(notice,"projected state : "<<model::displayConfig(configuration));
    if(res.success_)
    {
        std::vector<std::string> effNames(extractEffectorsName(fullBody->GetLimbs()));
        std::vector<std::string> freeLimbs = rbprm::freeEffectors(currentState,effNames.begin(), effNames.end() );
        for(std::vector<std::string>::const_iterator cit = freeLimbs.begin(); cit != freeLimbs.end() && res.success_; ++cit)
        {
            //hppDout(notice,"free effector in projection : "<<*cit);
            tmp = projection::setCollisionFree(fullBody,fullBody->GetLimbCollisionValidation().at(*cit),*cit,res.result_);
            if(!tmp.success_)
                res.success_ = false;
        }
    }
    //hppDout(notice,"project to col free, set coll free success = "<<res.success_);
    if(res.success_)
    {
        res.success_ = proj->apply(configuration);
        res.result_.configuration_ = configuration;
        if(res.success_)
        {
            ValidationReportPtr_t report (ValidationReportPtr_t(new CollisionValidationReport));
            res.success_ = fullBody->GetCollisionValidation()->validate(configuration,report);
            //hppDout(notice,"project to col free, collision test : "<<res.success_);
            if(!res.success_){
              CollisionValidationReportPtr_t repCast = boost::dynamic_pointer_cast<CollisionValidationReport>(report);
              //hppDout(notice,"collision between "<<repCast->object1->name()<<" and "<<repCast->object2->name());
            }
        }
    }
    return res;
}


} // namespace projection
} // namespace rbprm
} // namespace hpp
