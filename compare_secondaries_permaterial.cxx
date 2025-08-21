#include <TFile.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TLegend.h>
#include <TStyle.h>

void compare_secondaries_permaterial() {
    // === Archivos de entrada ===
    TFile *f1 = TFile::Open("secondaries_permaterial_RunD121_HGCal_new_cuts_FTFP_BERT.root");
    TFile *f2 = TFile::Open("secondaries_permaterial_RunD121_HGCal_original_cuts_FTFP_BERT_EMZ.root");

    if (!f1 || !f2) { 
        printf("Error abriendo los archivos.\n"); 
        return; 
    }

    // === Histogramas 2D ===
    TH2D *h2_newcuts = (TH2D*) f1->Get("fh2ZEdepMat");
    TH2D *h2_emz     = (TH2D*) f2->Get("fh2ZEdepMat");

    if (!h2_newcuts || !h2_emz) {
        printf("Error: no se encontró fh2ZEdepMat en los ROOT files.\n");
        return;
    }

    // === Rango en X (Z) para la proyección ===
//     int binXmin = h2_newcuts->GetXaxis()->FindBin(100.0); // ejemplo: desde Z=1000 mm
//     int binXmax = h2_newcuts->GetXaxis()->FindBin(900.0); // hasta Z=2000 mm
    int binXmin = h2_newcuts->GetXaxis()->FindBin(245.0); // ejemplo: desde Z=1000 mm
    int binXmax = h2_newcuts->GetXaxis()->FindBin(265.0); // hasta Z=2000 mm

    // === Proyección en Y ===
    TH1D *proj_newcuts = h2_newcuts->ProjectionY("proj_newcuts", binXmin, binXmax);
    TH1D *proj_emz     = h2_emz->ProjectionY("proj_emz", binXmin, binXmax);

    // Normalizar (opcional, según lo que quieras comparar)
//     proj_newcuts->Scale(1.0 / proj_newcuts->Integral());
//     proj_emz->Scale(1.0 / proj_emz->Integral());

    // === Diferencia relativa: (newcuts - emz)/emz ===
    TH1D *h_diff = (TH1D*) proj_newcuts->Clone("h_diff");
    h_diff->Add(proj_emz, -1);          // newcuts - emz
//     h_diff->Divide(proj_emz);           // (newcuts - emz)/emz
//     h_diff->SetTitle("Relative difference;Material; (newcuts - emz)/emz");
    h_diff->SetTitle("Abs difference;Material; (newcuts/EM0 - originalcut/EMZ)");

    // === Canvas con pads ===
    TCanvas *c = new TCanvas("c","Comparison",800,800);
    c->Divide(1,2);
    
    // Pad superior: proyecciones
    TPad *pad1 = (TPad*) c->cd(1);
    pad1->SetPad(0.0,0.3,1.0,1.0);
    pad1->SetBottomMargin(0.02);

    proj_newcuts->SetLineColor(kRed);
    proj_newcuts->SetMarkerColor(kRed);
    proj_newcuts->SetMarkerStyle(20);

    proj_emz->SetLineColor(kBlue);
    proj_emz->SetMarkerColor(kBlue);
    proj_emz->SetMarkerStyle(24);

    proj_newcuts->SetTitle("ProjectionY comparison;Material;Edep(MeV)");
    proj_newcuts->Draw("E1");
    proj_emz->Draw("E1 SAME");

    TLegend *leg = new TLegend(0.65,0.75,0.88,0.88);
    leg->AddEntry(proj_newcuts,"New cuts + EM0","lep");
    leg->AddEntry(proj_emz,"Original cuts + EMZ","lep");
    leg->Draw();

    // Pad inferior: diferencia relativa
    TPad *pad2 = (TPad*) c->cd(2);
    pad2->SetPad(0.0,0.0,1.0,0.3);
    pad2->SetTopMargin(0.02);
    pad2->SetBottomMargin(0.3);

    h_diff->SetLineColor(kBlack);
    h_diff->SetMarkerStyle(21);
    h_diff->GetYaxis()->SetTitleSize(0.08);
    h_diff->GetYaxis()->SetTitleOffset(0.5);
    h_diff->GetYaxis()->SetLabelSize(0.07);
    h_diff->GetXaxis()->SetTitleSize(0.1);
    h_diff->GetXaxis()->SetLabelSize(0.08);

    h_diff->Draw("E1");

    c->cd();
//     c->SaveAs("comparison_projectionY.png");

    for( int i = 0; i<h_diff->GetNbinsX(); ++i)
        std::cout << h_diff->GetXaxis()->GetBinLabel(i+1) << "\t" << h_diff->GetBinContent(i+1) << std::endl;
}

