/*
 * OnOffPolicyPredictionTest.cpp
 *
 *  Created on: Oct 26, 2013
 *      Author: sam
 */

#include "OnOffPolicyPredictionTest.h"

void OnOffPolicyPredictionTest::testTD(FSGAgentState* agentState, FiniteStateGraph* graph,
    OnPolicyTD<double>* td, const int& nbEpisodeMax, const double& precision)
{
  Timer timer;
  timer.start();
  int nbEpisode = 0;
  const Vector<double>* solution = graph->expectedDiscountedSolution();
  Vector<double>* x_t = new PVector<double>(agentState->dimension());
  Vector<double>* x_tp1 = new PVector<double>(agentState->dimension());
  while (FiniteStateGraph::distanceToSolution(solution, td->weights()) > precision)
  {
    FiniteStateGraph::StepData stepData = agentState->step();
    //x_t->set(agentState->project(stepData.v_t()));
    x_tp1->set(agentState->currentFeatureState());
    if (stepData.v_t()->empty())
      td->initialize();
    else
      td->update(x_t, x_tp1, stepData.r_tp1);
    if (stepData.s_tp1->v()->empty())
    {
      ++nbEpisode;
      //double error = distanceToSolution(solution, td->weights());
      //std::cout << "nbEpisode=" << nbEpisode << " error=" << error << std::endl;
      if (nbEpisode > nbEpisodeMax)
        break;
    }
    x_t->set(x_tp1);
    Assert::checkValues(td->weights());
  }
  timer.stop();

  double error = FiniteStateGraph::distanceToSolution(solution, td->weights());
  std::cout << "## nbEpisode=" << nbEpisode << " error=" << error << " elapsedTime(ms)="
      << timer.getElapsedTimeInMilliSec() << std::endl;

  delete x_t;
  delete x_tp1;
}

void OnOffPolicyPredictionTest::testOffPolicyGTD(FSGAgentState* agentState, RandomWalk* problem,
    OffPolicyTD<double>* gtd, const int& nbEpisodeMax, const double& precision,
    const double& targetLeftProbability, const double& behaviourLeftProbability)
{
  Timer timer;
  timer.start();
  int nbEpisode = 0;
  Policy<double>* behaviorPolicy = RandomWalk::newPolicy(problem->getActions(), behaviourLeftProbability);
  Policy<double>* targetPolicy = RandomWalk::newPolicy(problem->getActions(), targetLeftProbability);
  problem->setPolicy(behaviorPolicy);

  const Vector<double>* solution = problem->expectedDiscountedSolution();
  Vector<double>* phi_t = new PVector<double>(agentState->dimension());
  Vector<double>* phi_tp1 = new PVector<double>(agentState->dimension());
  while (FiniteStateGraph::distanceToSolution(solution, gtd->weights()) > precision)
  {
    FiniteStateGraph::StepData stepData = agentState->step();
    //phi_t->set(agentState->project(stepData.v_t()));
    phi_tp1->set(agentState->currentFeatureState());
    if (stepData.v_t()->empty())
      gtd->initialize();
    else
    {
      double pi_t = targetPolicy->pi(stepData.a_t);
      double b_t = behaviorPolicy->pi(stepData.a_t);
      double rho_t = pi_t / b_t;
      gtd->update(phi_t, phi_tp1, rho_t, stepData.r_tp1, 0.0);
    }
    if (stepData.s_tp1->v()->empty())
    {
      ++nbEpisode;
      //double error = distanceToSolution(solution, td->weights());
      //std::cout << "nbEpisode=" << nbEpisode << " error=" << error << std::endl;
      if (nbEpisode > nbEpisodeMax)
        break;
    }
    phi_t->set(phi_tp1);
    Assert::checkValues(gtd->weights());
  }
  timer.stop();

  double error = FiniteStateGraph::distanceToSolution(solution, gtd->weights());
  std::cout << "## nbEpisode=" << nbEpisode << " error=" << error << " elapsedTime(ms)="
      << timer.getElapsedTimeInMilliSec() << std::endl;

  delete targetPolicy;
  delete phi_t;
  delete phi_tp1;
}

void OnOffPolicyPredictionTest::testOnLineProblem()
{
  {
    FiniteStateGraph* graph = new LineProblem;
    FSGAgentState* agentState = new FSGAgentState(graph);
    OnPolicyTD<double>* td = new TD<double>(0.01 / agentState->vectorNorm(), graph->gamma(),
        agentState->dimension());
    testTD(agentState, graph, td, 100000, 0.01);

    delete graph;
    delete agentState;
    delete td;
  }

  {
    FiniteStateGraph* graph = new LineProblem;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new ATrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new TDLambda<double>(0.01 / agentState->vectorNorm(), graph->gamma(),
        0.5, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }

  {
    FiniteStateGraph* graph = new LineProblem;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new ATrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new TDLambda<double>(0.01 / agentState->vectorNorm(), graph->gamma(),
        1.0, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }

  {
    FiniteStateGraph* graph = new LineProblem;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.1, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }

  {
    FiniteStateGraph* graph = new LineProblem;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.2, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }
}

void OnOffPolicyPredictionTest::testOnRandomWalkProblem()
{
  //srand(0); // For consistency
  {
    FiniteStateGraph* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    OnPolicyTD<double>* td = new TD<double>(0.01 / agentState->vectorNorm(), graph->gamma(),
        agentState->dimension());
    testTD(agentState, graph, td, 100000, 0.01);

    delete graph;
    delete agentState;
    delete td;
  }

  {
    FiniteStateGraph* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new ATrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new TDLambda<double>(0.01 / agentState->vectorNorm(), graph->gamma(),
        0.5, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }

  {
    FiniteStateGraph* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new ATrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new TDLambda<double>(0.01 / agentState->vectorNorm(), graph->gamma(),
        1.0, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }

  {
    FiniteStateGraph* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.1, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }

  {
    FiniteStateGraph* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OnPolicyTD<double>* td = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.2, e);
    testTD(agentState, graph, td, 100000, 0.01);
    delete graph;
    delete agentState;
    delete e;
    delete td;
  }
}

void OnOffPolicyPredictionTest::testOffPolicy()
{

  {
    RandomWalk* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OffPolicyTD<double>* gtd = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.0, e);
    testOffPolicyGTD(agentState, graph, gtd, 100000, 0.05, 0.2, 0.5);
    delete graph;
    delete agentState;
    delete e;
    delete gtd;
  }

  {
    RandomWalk* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OffPolicyTD<double>* gtd = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.0, e);
    testOffPolicyGTD(agentState, graph, gtd, 100000, 0.05, 0.5, 0.2);
    delete graph;
    delete agentState;
    delete e;
    delete gtd;
  }

}

void OnOffPolicyPredictionTest::testOffPolicyWithLambda()
{
  {
    RandomWalk* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OffPolicyTD<double>* gtd = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.1, e);
    testOffPolicyGTD(agentState, graph, gtd, 100000, 0.05, 0.2, 0.5);
    delete graph;
    delete agentState;
    delete e;
    delete gtd;
  }

  {
    RandomWalk* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OffPolicyTD<double>* gtd = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.1, e);
    testOffPolicyGTD(agentState, graph, gtd, 100000, 0.05, 0.5, 0.2);
    delete graph;
    delete agentState;
    delete e;
    delete gtd;
  }

  {
    RandomWalk* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OffPolicyTD<double>* gtd = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.2, e);
    testOffPolicyGTD(agentState, graph, gtd, 100000, 0.05, 0.2, 0.5);
    delete graph;
    delete agentState;
    delete e;
    delete gtd;
  }

  {
    RandomWalk* graph = new RandomWalk;
    FSGAgentState* agentState = new FSGAgentState(graph);
    Trace<double>* e = new AMaxTrace<double>(agentState->dimension());
    OffPolicyTD<double>* gtd = new GTDLambda<double>(0.01 / agentState->vectorNorm(),
        0.5 / agentState->vectorNorm(), graph->gamma(), 0.2, e);
    testOffPolicyGTD(agentState, graph, gtd, 100000, 0.05, 0.5, 0.2);
    delete graph;
    delete agentState;
    delete e;
    delete gtd;
  }

}

void OnOffPolicyPredictionTest::run()
{
  testOnLineProblem();
  testOnRandomWalkProblem();
  testOffPolicy();
  testOffPolicyWithLambda();
}

RLLIB_TEST_MAKE(OnOffPolicyPredictionTest)
