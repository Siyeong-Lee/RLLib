/*
 * NoisyInputSum.h
 *
 *  Created on: Nov 18, 2013
 *      Author: sam
 */

#ifndef NOISYINPUTSUM_H_
#define NOISYINPUTSUM_H_

#include "Vector.h"
#include "Math.h"
#include "Supervised.h"

namespace RLLib
{

template<class T>
class PredictionProblem
{
  public:
    virtual ~PredictionProblem()
    {
    }
    virtual bool update() =0;
    virtual T getTarget() const =0;
    virtual const Vector<T>* getInputs() const =0;
};

class NoisyInputSum: public PredictionProblem<double>
{
  protected:
    double target;
    int nbChangingWeights;
    int changePeriod;
    int nbSteps;
    Vector<double>* inputs;
    Vector<double>* w;

  public:
    NoisyInputSum(const int& nbNonZeroWeights, const int& nbInputs) :
        target(0), nbChangingWeights(nbNonZeroWeights), changePeriod(20), nbSteps(0), inputs(
            new PVector<double>(nbInputs)), w(new PVector<double>(nbInputs))
    {
      for (int i = 0; i < w->dimension(); i++)
      {
        if (i < nbNonZeroWeights)
          w->setEntry(i, Probabilistic::nextDouble() > 0.5f ? 1.0f : -1.0f);
      }
    }

    virtual ~NoisyInputSum()
    {
      delete inputs;
      delete w;
    }

  private:
    void changeWeight()
    {
      const int i = Probabilistic::nextInt(nbChangingWeights);
      w->setEntry(i, w->getEntry(i) * -1.0f);
    }

  public:
    bool update()
    {
      ++nbSteps;
      if ((nbSteps % changePeriod) == 0)
        changeWeight();
      inputs->clear();
      for (int i = 0; i < inputs->dimension(); i++)
        inputs->setEntry(i, Probabilistic::nextNormalGaussian());
      target = w->dot(inputs);
      return true;
    }

    double getTarget() const
    {
      return target;
    }

    const Vector<double>* getInputs() const
    {
      return inputs;
    }
};

class NoisyInputSumEvaluation
{
  public:
    int nbInputs;
    int nbNonZeroWeights;

    NoisyInputSumEvaluation() :
        nbInputs(20), nbNonZeroWeights(5)
    {
    }

    double evaluateLearner(LearningAlgorithm<double>* algorithm, const int& learningEpisodes,
        const int& evaluationEpisodes)
    {
      NoisyInputSum noisyInputSum(nbNonZeroWeights, nbInputs);
      Timer timer;
      timer.start();
      for (int i = 0; i < learningEpisodes; i++)
      {
        noisyInputSum.update();
        algorithm->learn(noisyInputSum.getInputs(), noisyInputSum.getTarget());
      }
      timer.stop();
      double elapsedTime = timer.getElapsedTimeInMilliSec();
      PVector<double> errors(evaluationEpisodes);
      timer.start();
      for (int i = 0; i < evaluationEpisodes; i++)
      {
        noisyInputSum.update();
        errors[i] = algorithm->learn(noisyInputSum.getInputs(), noisyInputSum.getTarget());
        Boundedness::checkValue(errors[i]);
      }
      timer.stop();
      elapsedTime += timer.getElapsedTimeInMilliSec();
      std::cout << "Time=" << (elapsedTime / (learningEpisodes + evaluationEpisodes)) << std::endl;
      double mrse = errors.dot(&errors) / errors.dimension();
      Boundedness::checkValue(mrse);
      return mrse;
    }

    double evaluateLearner(LearningAlgorithm<double>* algorithm)
    {
      return evaluateLearner(algorithm, 20000, 10000);
    }
};

}  // namespace RLLib

#endif /* NOISYINPUTSUM_H_ */