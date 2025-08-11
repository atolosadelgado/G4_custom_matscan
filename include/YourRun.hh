#ifndef YOURRUN_HH
#define YOURRUN_HH

#include "G4Run.hh"

#include <tools/histo/h1d>
#include <vector>
#include <memory>

class YourRun : public G4Run {
  public:

    // CTR:
    YourRun();
    ~YourRun() override;


    /**
     * Virtual method to be implemented to define the way of merging the underlying
     * (thread local) Run global data structures into one global instance.
     *
     * This method will be invoked by the master on its own run object (YourRun)
     * on each of the worker, i.e. thread local, run (YourRun) objects passing by their
     * pointer as the input argument.
     *
     * @param [in] run Pointer to a run (YourRun) obejct that needs to be merged to this run object.
     */
    void Merge(const G4Run* run) override;


    //
    // Additional custom methods:


    // Method that we will call from the master run action at the end to compute final quantities using the
    // its run global Run object into which all thread local run global run object instance has already been merged
    // (when YourRunAction::EndOfRunAction() is invoked).
    void  EndOfRunSummary();




  // Data member declarations:
  private:
    std::vector<std::unique_ptr<tools::histo::h1d>> fHistos;


};

#endif
