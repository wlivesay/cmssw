// plot_z0_ratio.C
//
// Restructures an existing canvas (already drawn by the overlay macro) to add
// a ratio subplot beneath it.  Does NOT save -- the overlay macro's SaveAs
// call captures the full canvas including the ratio pad.
//
// Call this BEFORE c.SaveAs(...) in your overlay macro:
//
//   #include "Billy/overlay/subplot.C"
//   plot_z0_ratio(h1, h2, &c);
//   c.SaveAs("Billy/overlay/outputs/overlay_"+f1+"_vs_"+f2+"_"+what+".pdf");

#include "TH1.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLine.h"
#include "TBox.h"
#include "TStyle.h"
#include "TMath.h"
#include "TString.h"
#include "TList.h"
#pragma once
// ── Colour for the ratio points (matches your overlay's first histogram) ─────
static const int COLOR_RATIO = kRed + 1;


// ─────────────────────────────────────────────────────────────────────────────
// Helper: bin-by-bin ratio with Gaussian error propagation
// ─────────────────────────────────────────────────────────────────────────────
TH1F* make_ratio(TH1F* hist_a, TH1F* hist_b)
{
    TH1F* ratio = (TH1F*) hist_a->Clone("h_z0_ratio");
    ratio->Reset();
    ratio->SetDirectory(nullptr);

    for (int i = 1; i <= hist_a->GetNbinsX(); ++i) {
        double a  = hist_a->GetBinContent(i);
        double b  = hist_b->GetBinContent(i);
        double ea = hist_a->GetBinError(i);
        double eb = hist_b->GetBinError(i);

        if (b == 0) continue;

        double r    = a / b;
        double rerr = r * TMath::Sqrt((a != 0 ? (ea/a)*(ea/a) : 0) +
                                      (eb/b)*(eb/b));
        ratio->SetBinContent(i, r);
        ratio->SetBinError(i, rerr);
    }
    return ratio;
}


// ─────────────────────────────────────────────────────────────────────────────
// Main function
//   hist_a, hist_b : the same two TH1F* you passed to the overlay macro
//   canvas         : pointer to the canvas your overlay macro drew on (&c)
// ─────────────────────────────────────────────────────────────────────────────
void plot_z0_ratio(TH1F*    hist_a,
                   TH1F*    hist_b,
                   TCanvas* canvas)
{
    TH1F* h_ratio = make_ratio(hist_a, hist_b);


    // ── Grab the existing main pad from the canvas ────────────────────────────
    canvas->cd();

    TPad* pad_main = new TPad("pad_main", "", 0.0, 0.28, 1.0, 1.0);
    pad_main->SetBottomMargin(0.02);
    pad_main->SetTopMargin(0.08);
    pad_main->SetLeftMargin(0.12);
    pad_main->SetRightMargin(0.05);
    pad_main->SetGridy(canvas->GetGridy());  // preserve your SetGridy call

    // Move every primitive from the canvas onto pad_main
    TList* prims = canvas->GetListOfPrimitives();
    TList* to_move = new TList();
    TIter next(prims);
    TObject* obj;
    while ((obj = next())) {
        if (!obj->InheritsFrom(TPad::Class()))
            to_move->Add(obj);
    }
    TIter next2(to_move);
    while ((obj = next2())) {
        prims->Remove(obj);
        pad_main->GetListOfPrimitives()->Add(obj);
    }
    delete to_move;

    // Suppress the x-axis labels on the main pad (ratio pad carries them)
    TIter next3(pad_main->GetListOfPrimitives());
    while ((obj = next3())) {
        if (obj->InheritsFrom(TH1::Class())) {
            ((TH1*)obj)->GetXaxis()->SetLabelSize(0);
            ((TH1*)obj)->GetXaxis()->SetTitleSize(0);
            break;
        }
    }

    canvas->GetListOfPrimitives()->Add(pad_main);
    pad_main->Draw();


    // ── Ratio pad ─────────────────────────────────────────────────────────────
    canvas->cd();
    TPad* pad_ratio = new TPad("pad_ratio", "", 0.0, 0.0, 1.0, 0.28);
    pad_ratio->SetTopMargin(0.02);
    pad_ratio->SetBottomMargin(0.32);
    pad_ratio->SetLeftMargin(0.12);
    pad_ratio->SetRightMargin(0.05);
    pad_ratio->Draw();
    pad_ratio->cd();

    h_ratio->SetLineColor(COLOR_RATIO);
    h_ratio->SetMarkerColor(COLOR_RATIO);
    h_ratio->SetMarkerStyle(20);
    h_ratio->SetMarkerSize(0.8);

    h_ratio->GetYaxis()->SetRangeUser(0.80, 1.20);
    h_ratio->GetYaxis()->SetTitle("Ratio black/red");
    h_ratio->GetYaxis()->SetNdivisions(505);
    h_ratio->GetYaxis()->SetTitleSize(0.12);
    h_ratio->GetYaxis()->SetTitleOffset(0.45);
    h_ratio->GetYaxis()->SetLabelSize(0.10);

    h_ratio->GetXaxis()->SetTitle("#eta");
    h_ratio->GetXaxis()->SetTitleSize(0.14);
    h_ratio->GetXaxis()->SetTitleOffset(0.88);
    h_ratio->GetXaxis()->SetLabelSize(0.11);

    h_ratio->Draw("E1");

    // Unity line
    double xlo = h_ratio->GetXaxis()->GetXmin();
    double xhi = h_ratio->GetXaxis()->GetXmax();
    TLine* line = new TLine(xlo, 1.0, xhi, 1.0);
    line->SetLineColor(kGray + 1);
    line->SetLineStyle(2);
    line->SetLineWidth(1);
    line->Draw();

    // ±5 % reference band
    TBox* band = new TBox(xlo, 0.95, xhi, 1.05);
    band->SetFillColorAlpha(kGray, 0.25);
    band->SetFillStyle(1001);
    band->Draw();
    h_ratio->Draw("E1 SAME");   // redraw points on top of band

    // Return focus to canvas so SaveAs in the overlay file works normally
    canvas->cd();
}
