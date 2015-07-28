void CalculateEfficiencies(std::string name) {

  const double x1 = -6000;
  const double x2 = 6000;
  const double y1 = -3000;
  const double y2 = 2500;
  const double maxDist = 7;
  const char* matchPlot = "Matching/DUTDiamondDistance";
  const char* effPlot = "Efficiency/DUTDiamondMapC";

  TEfficiency* efficiency = 0;
  double ec, eu, el;
  double corr, corrErr;

  JudithUtils::CalculateBackground(
    corr, corrErr, maxDist, name.c_str(), matchPlot
  );

  efficiency = JudithUtils::GetEfficiency(name.c_str(), effPlot);
  JudithUtils::CalculateEfficiency(
    efficiency, ec, eu, el, corr, corrErr, x1, x2, y1, y2
  );

  cout << "Efficiency : " << ec << "^{+" << eu << "}_{-" << el << "}" << endl;
}
