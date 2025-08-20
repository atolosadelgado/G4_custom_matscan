#ifndef MYPRIMARYGENERATOR
#define MYPRIMARYGENERATOR

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    MyPrimaryGenerator(): G4VUserPrimaryGeneratorAction() { fParticleGun = new G4ParticleGun(1); }
    ~MyPrimaryGenerator(){delete fParticleGun;}
    virtual void GeneratePrimaries(G4Event* anEvent){
        if( counter % 10 == 0)
        std::cout << "Event " << counter << std::endl;
        counter++;
        fParticleGun->GeneratePrimaryVertex(anEvent);
    }
private:
    G4ParticleGun* fParticleGun;
    int counter = {0};
};

#endif

