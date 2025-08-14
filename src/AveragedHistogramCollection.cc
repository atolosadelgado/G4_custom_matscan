
#include "G4Material.hh"


#include <AveragedHistogramCollection.hh>

void AveragedHistogramCollection::Initialize(std::string histo_basename, int nbins_input, double zmin_input, double zmax_input)
{
  G4MaterialTable* material_table_vector = G4Material::GetMaterialTable();
  if(nullptr == material_table_vector) throw std::runtime_error("Material table not found, null pointer!");

  nbins = nbins_input;
  zmin = zmin_input;
  zmax = zmax_input;

  for( auto material : *material_table_vector){

    if(nullptr == material) throw std::runtime_error("Material not found, null pointer!");

    std::string mat_name = material->GetName();

    // skip materials that make up universe or laboratory, Galactic/air respectively
    // if ("Galactic" == mat_name || "Air" == mat_name) continue;

      const std::string histo_name = histo_basename + "_" + mat_name;
      averagedProfilePerMaterial_map.emplace(
          std::piecewise_construct,
          std::forward_as_tuple(material),
          std::forward_as_tuple(histo_name, nbins, zmin, zmax)
      );
      // std::cout << "\t-New histogram: " << histo_name << std::endl;
  }

}

void AveragedHistogramCollection::UpdateAverageAndMean(const tools::histo::h1d& h, G4Material* mat)
{
    if( nullptr == mat) return;
    auto map_iterator = this->averagedProfilePerMaterial_map.find(mat);
    if( this->averagedProfilePerMaterial_map.end() != map_iterator)
    {
      map_iterator->second.update( h );
    }
}

void AveragedHistogramCollection::SaveRootfile(std::string ofilename)
{
    if( 0 == ofilename.size()) return;

    TFile * ofile = TFile::Open(ofilename.c_str(), "recreate");

    for( auto & eProfileForMaterial : this->averagedProfilePerMaterial_map)
        eProfileForMaterial.second.finalize(ofile);

    ofile->Close();
}

void AveragedHistogramCollection::ResetHistograms()
{
    for( auto & eProfileForMaterial : this->averagedProfilePerMaterial_map)
        eProfileForMaterial.second.reset();
}

void AveragedHistogramCollection::ClearMapOfHistograms()
{
    this->averagedProfilePerMaterial_map.clear();
}


