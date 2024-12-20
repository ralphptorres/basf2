/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/utility/CDCDedxSigmaPred.h>

using namespace Belle2;

void CDCDedxSigmaPred::setParameters()
{

  // make sure the resolution parameters are reasonable
  if (!m_DBSigmaPars || m_DBSigmaPars->getSize() == 0)
    B2FATAL("No dE/dx sigma parameters!");

  std::vector<double> sigmapar;
  sigmapar = m_DBSigmaPars->getSigmaPars();
  for (int i = 0; i < 2; ++i)  m_dedxpars[i] = sigmapar[i];
  for (int i = 0; i < 5; ++i) m_nhitpars[i] = sigmapar[i + 2];
  for (int i = 0; i < 10; ++i) m_cospars[i] = sigmapar[i + 7];
}

void CDCDedxSigmaPred::setParameters(std::string infile)
{

  B2INFO("\n\tWidgetParameterization: Using parameters from file --> " << infile);

  std::ifstream fin;
  fin.open(infile.c_str());

  if (!fin.good()) B2FATAL("\tWARNING: CANNOT FIND " << infile);

  int par;

  B2INFO("\t --> dedx parameters");
  for (int i = 0; i < 2; ++i) {
    fin >> par >> m_dedxpars[i];
    B2INFO("\t\t (" << i << ")" << m_dedxpars[i]);
  }

  B2INFO("\t --> nhit parameters");
  for (int i = 0; i <= 4; ++i) {
    fin >> par >> m_nhitpars[i];
    B2INFO("\t\t (" << i << ")" << m_nhitpars[i]);
  }

  B2INFO("\t --> cos parameters");
  for (int i = 0; i < 10; ++i) {   //4
    fin >> par >> m_cospars[i];
    B2INFO("\t\t (" << i << ")" << m_cospars[i]);
  }

  fin.close();
}

void CDCDedxSigmaPred::printParameters(std::string outfile)
{

  B2INFO("\n\tCDCDedxSigmaPred: Printing parameters to file --> " << outfile.c_str());

  // write out the parameters to file
  std::ofstream fout(outfile.c_str());

  for (int i = 1; i < 3; ++i) fout << i << "\t" << m_dedxpars[i - 1] << std::endl;

  fout << std::endl;
  for (int i = 3; i < 8; ++i) fout << i << "\t" << m_nhitpars[i - 3] << std::endl;

  fout << std::endl;
  for (int i = 8; i < 18; ++i) fout << i << "\t" << m_cospars[i - 8] << std::endl;

  fout << std::endl;

  fout.close();
}

double CDCDedxSigmaPred::getSigma(double dedx, double nhit, double cos, double timereso)
{
  double correction  = cosPrediction(cos) * nhitPrediction(nhit) * ionzPrediction(dedx) * timereso;
  return correction;
}


double CDCDedxSigmaPred::nhitPrediction(double nhit)
{

  double x[1];
  double nhitpar[6];

  nhitpar[0] = 2;
  for (int i = 0; i < 5; ++i) nhitpar[i + 1] = m_nhitpars[i];

  // determine sigma from the nhit parameterization
  CDCDedxWidgetSigma gs;
  x[0] = nhit;

  double corNHit;
  int nhit_min = 8, nhit_max = 37;

  if (nhit <  nhit_min) {
    x[0] = nhit_min;
    corNHit = gs.sigmaCurve(x, nhitpar) * sqrt(nhit_min / nhit);
  } else if (nhit > nhit_max) {
    x[0] = nhit_max;
    corNHit = gs.sigmaCurve(x, nhitpar) * sqrt(nhit_max / nhit);
  } else corNHit = gs.sigmaCurve(x, nhitpar);

  return corNHit;
}

double CDCDedxSigmaPred::ionzPrediction(double dedx)
{

  double x[1];
  double dedxpar[3];

  dedxpar[0] = 1;

  for (int i = 0; i < 2; ++i) dedxpar[i + 1] = m_dedxpars[i];

  // determine sigma from the parameterization
  CDCDedxWidgetSigma gs;
  x[0] = dedx;
  double corDedx = gs.sigmaCurve(x, dedxpar);

  return corDedx;
}

double CDCDedxSigmaPred::cosPrediction(double cos)
{

  double x[1];
  double corCos;
  CDCDedxWidgetSigma gs;

  double cospar[11];
  cospar[0] = 3;

  for (int i = 0; i < 10; ++i)  cospar[i + 1] = m_cospars[i];

  x[0] = cos;
  corCos = gs.sigmaCurve(x, cospar);

  return corCos;

}
