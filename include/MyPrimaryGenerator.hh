#ifndef MYPRIMARYGENERATOR
#define MYPRIMARYGENERATOR

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4Event.hh"

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    MyPrimaryGenerator(): G4VUserPrimaryGeneratorAction() { fParticleGun = new G4ParticleGun(1); }
    ~MyPrimaryGenerator(){delete fParticleGun;}
    virtual void GeneratePrimaries(G4Event* anEvent){
        // if( counter % 10 == 0)
        // std::cout << "PrimaryGeneratorAction. Event " << counter << std::endl;
        // counter++;
        fParticleGun->GeneratePrimaryVertex(anEvent);
        x0_mm = anEvent->GetPrimaryVertex(0)->GetPosition().x() / CLHEP::mm;
        y0_mm = anEvent->GetPrimaryVertex(0)->GetPosition().y() / CLHEP::mm;
        E0_MeV = anEvent->GetPrimaryVertex(0)->GetPrimary()->GetKineticEnergy() / CLHEP::MeV;
    }
    G4double x0_mm{-999.};
    G4double y0_mm{-999.};
    G4double E0_MeV{-999.};

private:
    G4ParticleGun* fParticleGun;
    int counter = {0};
};

#endif

