#include "HistogramCollection.hh"

#include "G4Material.hh"

void HistogramCollection::Initialize(std::string histo_basename, int nbins_input, double zmin_input, double zmax_input){


    // if histograms already exist, only reset them
    if( 0 < histogramCollection_map.size() ){
        this->Reset();
        return;
    }

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
        histogramCollection_map.emplace(
            material,
            std::make_unique<tools::histo::h1d>(histo_name, nbins, zmin, zmax)
        );
  }

}

void HistogramCollection::Fill(double energy_in_MeV, double z_in_mm, G4Material* mat)
{
    if( nullptr == mat) return;
    auto map_iterator = this->histogramCollection_map.find(mat);
    if( this->histogramCollection_map.end() != map_iterator)
    {
        // fill histogram with weight "energy""
      map_iterator->second->fill( z_in_mm, energy_in_MeV );
    }
}

void HistogramCollection::Reset()
{
    for(auto & it : histogramCollection_map)
        it.second->reset();
}

