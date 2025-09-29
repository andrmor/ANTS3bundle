
#include "G4NCrystal/G4NCBias.hh"
#include "G4NCManager.hh"

#include "NCrystal/interfaces/NCRNG.hh"

#include "G4VDiscreteProcess.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VParticleChange.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4VTouchable.hh"
#include "G4NavigationHistory.hh"
#include "G4BOptnChangeCrossSection.hh"
#include "G4BiasingProcessInterface.hh"

namespace NC = NCrystal;
namespace NCG4 = G4NCrystal;

namespace G4NCrystal {
  namespace {
    class RNG_G4Wrapper : public NC::RNGStream {
      CLHEP::HepRandomEngine * m_engine;
    public:
      //Can be cheaply created on the stack just before being used in calls to
      //ProcImpl::Scatter objects, like:
      //
      //  RNG_G4Wrapper rng(G4Random::getTheEngine());
      //
      //Since G4Random::getTheEngine() always returns a thread-local engine,
      //this is then MT-safe!
      constexpr RNG_G4Wrapper(CLHEP::HepRandomEngine * e) noexcept : m_engine(e) {}
    protected:
      double actualGenerate() override { return m_engine->flat(); }
    };

  }
}


class NCG4::NCrystalBiasingOperator::NCrystalProcess final
  : public G4VDiscreteProcess
{
  // Process which delivers NCrystal physics for any materials with the
  // appropriate NCrystal property. It can for instance used with a
  // G4VBiasingOperation.

public:
  NCrystalProcess( const G4String& processName = "NCrystalProcess" );
  virtual ~NCrystalProcess();

  G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step& ) override;
  G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*) override;
  void BuildPhysicsTable(const G4ParticleDefinition&) override;
  G4bool IsApplicable(const G4ParticleDefinition&) override;
  void ResetNumberOfInteractionLengthLeft() override;

  //custom method used by biasing wrapper:
  G4bool passThrough( const G4Track& ) const;

private:
  G4ParticleChange m_particleChange;
  Manager * m_mgr;
};


NCG4::NCrystalBiasingOperator::
NCrystalProcess::~NCrystalProcess() = default;

NCG4::NCrystalBiasingOperator::
NCrystalProcess::NCrystalProcess( const G4String& processName)
  : G4VDiscreteProcess( processName ), m_mgr( Manager::getInstance() )
{
  if ( !m_mgr )
    G4Exception( "G4NCrystal::Process::GetMeanFreePath", "Error",
                 FatalException, "Could not get G4NCrystal manager" );
}

void NCG4::NCrystalBiasingOperator::
NCrystalProcess::BuildPhysicsTable(const G4ParticleDefinition&)
{
}

void NCG4::NCrystalBiasingOperator::
NCrystalProcess::ResetNumberOfInteractionLengthLeft()
{
  //TODO: why do we need this specialisation that does nothing??
  G4VDiscreteProcess::ResetNumberOfInteractionLengthLeft();
}

G4bool NCG4::NCrystalBiasingOperator::
NCrystalProcess::IsApplicable( const G4ParticleDefinition& pd )
{
  return pd.GetPDGEncoding() == 2112;
}

G4bool NCG4::NCrystalBiasingOperator::
NCrystalProcess::passThrough( const G4Track& trk ) const {

  if ( trk.GetParticleDefinition()->GetPDGEncoding() != 2112 )
    G4Exception( "G4NCrystal::Process::passThrough", "Error",
                 FatalException, "Only deals with neutrons" );

  if ( trk.GetKineticEnergy() > 5.0 * CLHEP::eV )
    return true;

  const G4Material* mat = trk.GetMaterial();
  auto matprop = trk.GetMaterial()->GetMaterialPropertiesTable();
  if ( !matprop || !matprop->ConstPropertyExists("NCScat") )
    return true;

  return false;//we will handle this!
}

G4double NCG4::NCrystalBiasingOperator::
NCrystalProcess::GetMeanFreePath(const G4Track& trk, G4double, G4ForceCondition*)
{
  if ( trk.GetParticleDefinition()->GetPDGEncoding() != 2112 )
    G4Exception( "G4NCrystal::Process::GetMeanFreePath", "Error",
                 FatalException, "Only deals with neutrons" );

  Manager::ProcAndCache procandcache =
    m_mgr->getScatterPropertyWithThreadSafeCache( trk.GetMaterial() );
  if ( !procandcache.first )
    G4Exception( "G4NCrystal::Process::GetMeanFreePath", "Error",
                 FatalException, "Could not get NCrystal Scatter process" );

  auto& ncscat = *procandcache.first;
  assert(procandcache.second!=nullptr);
  auto& cacheptr = *procandcache.second;

  double xs(0.0);
  try {
    constexpr double inv_eV = 1.0/CLHEP::eV;
    NC::NeutronEnergy nc_ekin_in{trk.GetKineticEnergy() * inv_eV};//NCrystal unit is eV
    const G4ThreeVector& indir = trk.GetMomentumDirection();
    if( ! ncscat.isOriented() ) {
      xs = ncscat.crossSection( cacheptr, nc_ekin_in,
                                NC::NeutronDirection{ indir.x(),
                                                      indir.y(),
                                                      indir.z() } ).get() * CLHEP::barn;
    } else {
      G4ThreeVector indir_local = trk.GetStep()->GetPreStepPoint()
        ->GetTouchable()->GetHistory()->GetTopTransform().TransformAxis(indir);
      xs = ncscat.crossSection( cacheptr, nc_ekin_in,
                                NC::NeutronDirection{ indir_local.x(),
                                                      indir_local.y(),
                                                      indir_local.z() } ).get() * CLHEP::barn;
    }
  } catch ( NC::Error::Exception& e ) {
    Manager::handleError("G4NCrystal::Process::GetMeanFreePath",102,e);//fixme 102??
  }

  return xs
      ? 1.0 / ( trk.GetMaterial()->GetTotNbOfAtomsPerVolume() * xs )
      : NC::kInfinity ;

}

G4VParticleChange* NCG4::NCrystalBiasingOperator::
NCrystalProcess::PostStepDoIt( const G4Track& trk, const G4Step& step )
{
  ClearNumberOfInteractionLengthLeft();

  if ( trk.GetParticleDefinition()->GetPDGEncoding() != 2112 )
    G4Exception( "G4NCrystal::Process::PostStepDoIt", "Error",
                 FatalException, "Only deals with neutrons" );

  Manager::ProcAndCache procandcache =
    m_mgr->getScatterPropertyWithThreadSafeCache( trk.GetMaterial() );
  if ( !procandcache.first )
    G4Exception( "G4NCrystal::Process::PostStepDoIt", "Error",
                 FatalException, "Could not get NCrystal Scatter process" );

 auto& ncscat = *procandcache.first;
 assert(procandcache.second!=nullptr);
 auto& cacheptr = *procandcache.second;

  G4ThreeVector g4outcome_dir;
  double g4outcome_ekin;
  try {
    auto random_engine = G4Random::getTheEngine();
    nc_assert(random_engine!=nullptr);
    RNG_G4Wrapper rng(random_engine);
    //NCrystal use eV:
    constexpr double inv_eV = 1.0/CLHEP::eV;
    NC::NeutronEnergy nc_ekin_in{trk.GetKineticEnergy() * inv_eV};
    const G4ThreeVector& indir = trk.GetMomentumDirection();
    if( ! ncscat.isOriented() ) {
      //Orientation of material does not matter:
      auto outcome = ncscat.sampleScatter(cacheptr,rng,nc_ekin_in,
                                          NC::NeutronDirection{ indir.x(),
                                                                indir.y(),
                                                                indir.z() } );
      g4outcome_ekin = outcome.ekin.get() * CLHEP::eV;
      g4outcome_dir.set(outcome.direction[0],outcome.direction[1],outcome.direction[2]);
    } else {
      //Orientation of material matters, need to transform to-and-from the frame
      //of the volume (touchable):
      const G4AffineTransform &trf = step.GetPreStepPoint()
        ->GetTouchable()->GetHistory()->GetTopTransform();
      G4ThreeVector indir_local = trf.TransformAxis(indir);
      auto outcome = ncscat.sampleScatter(cacheptr,rng,nc_ekin_in,
                                          NC::NeutronDirection{ indir_local.x(),
                                                                indir_local.y(),
                                                                indir_local.z() } );
      g4outcome_ekin = outcome.ekin.get() * CLHEP::eV;
      g4outcome_dir = trf.Inverse().TransformAxis( G4ThreeVector{outcome.direction[0],
                                                                 outcome.direction[1],
                                                                 outcome.direction[2]} );
    }
  } catch ( NC::Error::Exception& e ) {
    Manager::handleError("G4NCrystal::ProcWrapper::PostStepDoIt",101,e);
    //Avoid "may be used initialised" warnings from compilers that think we
    //continue here.
    g4outcome_ekin = 0.0;
    g4outcome_dir.set(0.0,0.0,0.0);
  }
  m_particleChange.Clear();
  m_particleChange.Initialize(trk);
  m_particleChange.ProposeWeight(trk.GetWeight());//a bit weird that we have to do this.
  m_particleChange.ProposeMomentumDirection( g4outcome_dir );
  m_particleChange.ProposeEnergy( g4outcome_ekin );
  return &m_particleChange;
}

namespace G4NCrystal {
  class NCrystalBiasingOperator::NCrystalFinalStateBiasOp final
    : public G4VBiasingOperation {
  public:
    NCrystalFinalStateBiasOp( const G4String name, NCrystalProcess* ncrystalProcess );
    virtual ~NCrystalFinalStateBiasOp() {};
    virtual G4VParticleChange*
    ApplyFinalStateBiasing( const G4BiasingProcessInterface* ,
                            const G4Track* , const G4Step* , G4bool& ) override;

    virtual const G4VBiasingInteractionLaw*
    ProvideOccurenceBiasingInteractionLaw( const G4BiasingProcessInterface* ,
                                           G4ForceCondition& ) override
    {
      return nullptr;
    }

    virtual G4double DistanceToApplyOperation( const G4Track* ,
                                               G4double ,
                                               G4ForceCondition* ) override
    {
      return DBL_MAX;
    }

    virtual G4VParticleChange* GenerateBiasingFinalState( const G4Track* ,
                                                          const G4Step* ) override
    {
      return nullptr;
    }
  private:
    NCrystalProcess* fNCrystalProcess = nullptr;
  };
}

NCG4::NCrystalBiasingOperator::NCrystalFinalStateBiasOp::
NCrystalFinalStateBiasOp( const G4String name,
                          NCrystalProcess* ncrystalProcess ) :
  fNCrystalProcess( ncrystalProcess ),
  G4VBiasingOperation( name )
{
}

G4VParticleChange* NCG4::NCrystalBiasingOperator::NCrystalFinalStateBiasOp::
ApplyFinalStateBiasing( const G4BiasingProcessInterface*,
                        const G4Track* track,
                        const G4Step* step,
                        G4bool& ) {
  return fNCrystalProcess->PostStepDoIt( *track, *step );
}

NCG4::NCrystalBiasingOperator::NCrystalBiasingOperator()
  : G4VBiasingOperator( "NCrystalBiasingOperator" )
{
  fNCrystalProcess = new NCrystalProcess;
  fCrossSectionBiasOp = new G4BOptnChangeCrossSection( "NCrystalCrossSection" );
  fNCrystalFinalStateBiasOp = new NCrystalFinalStateBiasOp( "NCrystalFinalState",
                                                            fNCrystalProcess );
}

NCG4::NCrystalBiasingOperator::~NCrystalBiasingOperator() = default;

bool NCG4::NCrystalBiasingOperator::applies( const G4Track* trk,
                                             const G4BiasingProcessInterface* bpi ) const
{
  return ( fNCrystalProcess
           && bpi
           && bpi->GetWrappedProcess()
           && bpi->GetWrappedProcess()->GetProcessName() == "hadElastic"
           && ! fNCrystalProcess->passThrough( *trk ) );
}

G4VBiasingOperation* NCG4::NCrystalBiasingOperator::
ProposeOccurenceBiasingOperation(  const G4Track* trk,
                                   const G4BiasingProcessInterface* bpi )
{
  assert( fNCrystalProcess );
  if ( ! applies( trk, bpi ) )
    return nullptr;
  double mfp = fNCrystalProcess->GetMeanFreePath( *trk, 0.0, nullptr );
  if ( !(mfp > 0.0) ) {
    G4Exception( "G4NCrystal::NCrystalBiasingOperator"
                 "::ProposeOccurenceBiasingOperation", "Error",
                 FatalException, "NCrystal returned non-finite cross section" );
  }
  fCrossSectionBiasOp->SetBiasedCrossSection( 1.0 / mfp );
  fCrossSectionBiasOp->Sample();
  return fCrossSectionBiasOp;
}

G4VBiasingOperation* NCG4::NCrystalBiasingOperator::
ProposeFinalStateBiasingOperation( const G4Track* trk,
                                   const G4BiasingProcessInterface* bpi )
{
  return applies( trk, bpi ) ? fNCrystalFinalStateBiasOp : nullptr;
}

G4VBiasingOperation* NCG4::NCrystalBiasingOperator::
ProposeNonPhysicsBiasingOperation( const G4Track* ,
                                   const G4BiasingProcessInterface* )
{
  return nullptr;
}
