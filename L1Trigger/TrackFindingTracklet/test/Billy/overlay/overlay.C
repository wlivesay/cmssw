#include "TROOT.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TMath.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TLine.h"
#include "TBox.h"
#include <iostream>
#include <string>
#include <vector>

//Billy
#include <fstream>
using namespace std;

void SetPlotStyle();
void mySmallText(Double_t x, Double_t y, Color_t color, const char *text);


// ----------------------------------------------------------------------------------------------------------------
// Billy added this new first function
std::string getTotalEff(const std::string& filename) {
  std::ifstream in(filename);
  std::string line;
  cout << filename << endl;
  while (std::getline(in, line)) {
    if (line.find("TOTAL efficiency") != std::string::npos) {
      size_t eq1 = line.find('=');
      if (eq1 == std::string::npos) continue;
      size_t eq2 = line.find('=', eq1 + 1);
      if (eq2 == std::string::npos) continue;
      std::string val = line.substr(eq1 + 1, eq2 - eq1 - 1);
      size_t start = val.find_first_not_of(" \t");
      if (start == std::string::npos) return "test";
      size_t end = val.find_last_not_of(" \t");
      val = val.substr(start, end - start + 1);
      return val;
    }
  }
  return "No text file!";
}

std::vector<TString> split(TString s, char delim = '_') {
  std::vector<TString> out;
  TObjArray* tokens = s.Tokenize(delim);
  for (int i = 0; i < tokens->GetEntries(); i++) {
    out.push_back(((TObjString*)tokens->At(i))->GetString());
  }
  delete tokens;
  return out;
}

TString diffLabel(TString f1, TString f2) {
  auto a = split(f1);
  auto b = split(f2);
  TString label = "";
  int n = std::min(a.size(), b.size());
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      TString part = a[i];
      part.ReplaceAll("HnKF", "Hybrid newKF");
      part.ReplaceAll("HDnKFK","HD newKF Kill");
      part.ReplaceAll("HDnKFM","HD newKF Merge");
      if (part == "HD") part = "Hybrid Displaced";
      if (part == "H") part = "Hybrid";
      if (label != "") label += " | ";
      label += part;
    }
  }
  return label;
}

TString buildLabel(TString fname) {
  auto p = split(fname);
  TString label = "";
  p[2].ReplaceAll("HnKF", "Hybrid newKF");
  p[2].ReplaceAll("HDnKFK","HD newKF Kill");
  p[2].ReplaceAll("HDnKFM","HD newKF Merge");
  if (p[2] == "HD") p[2] = "Hybrid Displaced";
  if (p[2] == "H") p[2] = "Hybrid";
  // This code above relies on Hybrid being the third string in the list!!!
  if (p.size() > 0) label += p[0];
  if (p.size() > 1) label += ", " + p[1];
  if (p.size() > 2) label += ", " + p[2];
  if (p.size() > 3) label += ", " + p[3];
  if (p.size() > 4) label += ", " + p[4];
  return label;
}

// ----------------------------------------------------------------------------------------------------------------
// Helper: draw all text labels (mySmallText calls) onto the current pad.
// Called once in both the "y" and "n" branches so the logic lives in one place.
// ----------------------------------------------------------------------------------------------------------------
void drawLabels(TString what, TString common, TString f1, TString f2,
                TH1F* h1, TH1F* h2,
                TString eff1, TString eff2) {
  if (what.Contains("eff")) {
    mySmallText(0.55, 0.52, 1, buildLabel(common).Data());
    if (!what.Contains("_H")   && !what.Contains("_L")  &&
        !what.Contains("_5")   && !what.Contains("_23") &&
        !what.Contains("_35")  && !what.Contains("_LC") &&
        !what.Contains("_eta2")) {
      mySmallText(0.55, 0.41, h1->GetLineColor(), TString("Total eff1 = ")+eff1+"%");
      mySmallText(0.55, 0.30, h2->GetLineColor(), TString("Total eff2 = ")+eff2+"%");
    }
  } else {
    mySmallText(0.15, 0.95, 1, buildLabel(common).Data());
  }
}

// ----------------------------------------------------------------------------------------------------------------
// Helper: build ratio histogram from h1/h2 with Gaussian error propagation.
// Caller owns the returned histogram.
// ----------------------------------------------------------------------------------------------------------------
TH1F* buildRatio(TH1F* h1, TH1F* h2) {
  TH1F* h_ratio = (TH1F*) h1->Clone("h_ratio");
  h_ratio->Reset();
  h_ratio->SetDirectory(nullptr);
  for (int i = 1; i <= h1->GetNbinsX(); ++i) {
    double a  = h1->GetBinContent(i);
    double b  = h2->GetBinContent(i);
    double ea = h1->GetBinError(i);
    double eb = h2->GetBinError(i);
    if (b == 0) continue;
    double r    = a / b;
    double rerr = r * TMath::Sqrt((a != 0 ? (ea/a)*(ea/a) : 0) + (eb/b)*(eb/b));
    h_ratio->SetBinContent(i, r);
    h_ratio->SetBinError(i, rerr);
  }
  return h_ratio;
}

// ----------------------------------------------------------------------------------------------------------------
// Helper: style and draw the ratio pad contents onto the current pad.
// ----------------------------------------------------------------------------------------------------------------
void drawRatioPad(TH1F* h_ratio) {
  h_ratio->SetLineColor(kBlack);
  h_ratio->SetMarkerColor(kBlack);
  h_ratio->SetMarkerStyle(8);
  //h_ratio->GetYaxis()->SetRangeUser(0.80, 1.20);
  double rmin = 1.0, rmax = 1.0;
  for (int i = 1; i <= h_ratio->GetNbinsX(); ++i) {
    double val = h_ratio->GetBinContent(i);
    double err = h_ratio->GetBinError(i);
    if (val == 0) continue;
    if (val - err < rmin) rmin = val - err;
    if (val + err > rmax) rmax = val + err;
  }
  double range = rmax - rmin;
  h_ratio->GetYaxis()->SetRangeUser(rmin - 0.1*range, rmax + 0.1*range);
  h_ratio->GetYaxis()->SetTitle("Ratio black/red");
  h_ratio->GetYaxis()->SetNdivisions(505);
  h_ratio->GetYaxis()->SetTitleSize(0.12);
  h_ratio->GetYaxis()->SetTitleOffset(0.45);
  h_ratio->GetYaxis()->SetLabelSize(0.10);
  h_ratio->GetXaxis()->SetTitleSize(0.14);
  h_ratio->GetXaxis()->SetTitleOffset(0.88);
  h_ratio->GetXaxis()->SetLabelSize(0.11);
  h_ratio->Draw("E1");

  double xlo = h_ratio->GetXaxis()->GetXmin();
  double xhi = h_ratio->GetXaxis()->GetXmax();

  TLine* line = new TLine(xlo, 1.0, xhi, 1.0);
  line->SetLineColor(kGray+1);
  line->SetLineStyle(2);
  line->Draw();

  h_ratio->Draw("E1 SAME");  // redraw points on top of band
}


// ----------------------------------------------------------------------------------------------------------------
// Main script
// ----------------------------------------------------------------------------------------------------------------
void overlay(TString what, TString file1, TString file2, TString include_subplot) {
  Ssiz_t pos1 = file1.Last('/');
  file1 = (pos1 != kNPOS) ? file1(pos1+1, file1.Length()-pos1-1) : file1;
  Ssiz_t pos2 = file2.Last('/');
  file2 = (pos2 != kNPOS) ? file2(pos2+1, file2.Length()-pos2-1) : file2;
  SetPlotStyle();

  TString setting = "";
  // Set to "counts" for max at 300 y-axis

  TFile* tree1 = new TFile("/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/"+file1);
  TFile* tree2 = new TFile("/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/"+file2);

  TH1F* h1 = (TH1F*) tree1->Get(what);
  TH1F* h2 = (TH1F*) tree2->Get(what);

  if (what.Contains("ntrk")) {
    h1->Rebin(10);
    h2->Rebin(10);
  }
  if (setting.Contains("count")) {
    h1->SetMaximum(300);
  }

  h1->SetLineColor(1);
  h1->SetMarkerColor(1);
  h1->SetMarkerStyle(8);
  h2->SetLineColor(2);
  h2->SetMarkerColor(2);
  h2->SetMarkerStyle(24);

  TString f1 = file1;
  TString f2 = file2;
  f1.ReplaceAll(".root", "");
  f2.ReplaceAll(".root", "");
  f1.ReplaceAll("output_", "");
  f2.ReplaceAll("output_", "");

  TString diff1 = diffLabel(f1, f2);
  TString diff2 = diffLabel(f2, f1);
  //std::cout << "diff1 is _" << diff1 << "_" << std::endl;
  //std::cout << "diff2 is _" << diff2 << "_" << std::endl;
  //std::cout << "diff1 is the value we try to take out of the string of all common terms!" << std::endl;

  TString common = f1;
  common.ReplaceAll("HnKF", "Hybrid newKF");
  common.ReplaceAll("HDnKFK","HD newKF Kill");
  common.ReplaceAll("HDnKFM","HD newKF Merge");
  common.ReplaceAll("_HD_","_Hybrid Displaced_");
  common.ReplaceAll("_H_","_Hybrid_");
  if (diff1 == "Hybrid")
    common.ReplaceAll("_Hybrid_","_");
  else if (diff1 == "Hybrid Displaced")
    common.ReplaceAll("_Hybrid Displaced_","_");
  else {
    common.ReplaceAll(diff1,"_");
    common.ReplaceAll("__","_");
  }

  // ── Compute efficiencies once so both branches can use them ───────────────
  TString eff1 = "", eff2 = "";
  if (what.Contains("eff") &&
      !what.Contains("_H")   && !what.Contains("_L")  &&
      !what.Contains("_5")   && !what.Contains("_23") &&
      !what.Contains("_35")  && !what.Contains("_LC") &&
      !what.Contains("_eta2")) {
    eff1 = getTotalEff(("/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/text_files/"+f1+"_Summary.txt").Data());
    eff2 = getTotalEff(("/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/text_files/"+f2+"_Summary.txt").Data());
  }

  // ── Legend (shared by both branches) ─────────────────────────────────────
  TLegend* l;
  if (what.Contains("eff")) {
    l = new TLegend(0.20, 0.22, 0.50, 0.40);
    if (what.Contains("Mu"))
      h1->GetYaxis()->SetRangeUser(0.8, 1.01);
  } else {
    l = new TLegend(0.2, 0.72, 0.5, 0.9);
  }
  l->SetFillColor(0);
  l->SetLineColor(0);
  l->SetTextSize(0.04);
  l->SetTextFont(42);
  l->AddEntry(h1, diff1, "lep");
  l->AddEntry(h2, diff2, "lep");

  // ── Draw ──────────────────────────────────────────────────────────────────
  if (include_subplot == "y") {
    TCanvas c("c","",800,700);

    TPad* pad_main = new TPad("pad_main","",0.0,0.28,1.0,1.0);
    pad_main->SetBottomMargin(0.02);
    pad_main->SetTopMargin(0.05);
    pad_main->SetLeftMargin(0.16);
    pad_main->SetRightMargin(0.05);
    pad_main->Draw();
    pad_main->cd();

    h1->GetXaxis()->SetLabelSize(0);
    h1->GetXaxis()->SetTitleSize(0);
    h1->Draw("lp");
    h2->Draw("lp,same");
    drawLabels(what, common, f1, f2, h1, h2, eff1, eff2);
    l->Draw();
    gPad->SetGridy();

    TPad* pad_ratio = new TPad("pad_ratio","",0.0,0.0,1.0,0.28);
    pad_ratio->SetTopMargin(0.02);
    pad_ratio->SetBottomMargin(0.32);
    pad_ratio->SetLeftMargin(0.16);
    pad_ratio->SetRightMargin(0.05);
    c.cd();
    pad_ratio->Draw();
    pad_ratio->cd();

    TH1F* h_ratio = buildRatio(h1, h2);
    drawRatioPad(h_ratio);
    c.cd();
    c.SaveAs("Billy/overlay/outputs/overlay_"+f1+"_vs_"+f2+"_"+what+".pdf");
    TFile("Billy/overlay/outputs/"+f1+"_vs_"+f2+"_ratio_"+what+".root", "RECREATE").WriteTObject(h_ratio);

  } else {
    TCanvas c;
    h1->Draw("lp");
    h2->Draw("lp,same");
    drawLabels(what, common, f1, f2, h1, h2, eff1, eff2);
    l->Draw();
    gPad->SetGridy();
    c.SaveAs("Billy/overlay/outputs/overlay_"+f1+"_vs_"+f2+"_"+what+".pdf");
  }
}


void SetPlotStyle() {
  // from ATLAS plot style macro
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameFillColor(0);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(0);
  gStyle->SetStatColor(0);
  gStyle->SetHistLineColor(1);
  gStyle->SetPalette(1);
  gStyle->SetPaperSize(20,26);
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadBottomMargin(0.16);
  gStyle->SetPadLeftMargin(0.16);
  gStyle->SetTitleXOffset(1.4);
  gStyle->SetTitleYOffset(1.4);
  gStyle->SetTextFont(42);
  gStyle->SetTextSize(0.05);
  gStyle->SetLabelFont(42,"x");
  gStyle->SetTitleFont(42,"x");
  gStyle->SetLabelFont(42,"y");
  gStyle->SetTitleFont(42,"y");
  gStyle->SetLabelFont(42,"z");
  gStyle->SetTitleFont(42,"z");
  gStyle->SetLabelSize(0.05,"x");
  gStyle->SetTitleSize(0.05,"x");
  gStyle->SetLabelSize(0.05,"y");
  gStyle->SetTitleSize(0.05,"y");
  gStyle->SetLabelSize(0.05,"z");
  gStyle->SetTitleSize(0.05,"z");
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetHistLineWidth(2.);
  gStyle->SetLineStyleString(2,"[12 12]");
  gStyle->SetEndErrorSize(0.);
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
}

void mySmallText(Double_t x, Double_t y, Color_t color, const char* text) {
  TLatex l;
  l.SetNDC();
  l.SetTextSize(0.044);
  l.SetTextColor(color);
  l.SetTextFont(42);
  l.DrawLatex(x, y, text);
}
