/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Heiko Strathmann, Soeren Sonnenburg, Sergey Lisitsyn, 
 *          Giovanni De Toni, Jacob Walker, Saurabh Mahindre, Yuyu Zhang, 
 *          Roman Votyakov
 */

#ifndef __CROSSVALIDATION_H_
#define __CROSSVALIDATION_H_

#include <shogun/lib/config.h>

#include <shogun/evaluation/EvaluationResult.h>
#include <shogun/evaluation/MachineEvaluation.h>

namespace shogun
{

	class CMachineEvaluation;
	class CCrossValidationOutput;
	class CrossValidationStorage;
	class CList;

	/** @brief type to encapsulate the results of an evaluation run.
	 */
	class CCrossValidationResult : public CEvaluationResult
	{
	public:
		CCrossValidationResult()
		{
			SG_ADD(&mean, "mean", "Mean of results", MS_NOT_AVAILABLE);
			SG_ADD(
			    &std_dev, "std_dev",
			    "Standard deviation of cross-validation folds",
			    MS_NOT_AVAILABLE);

			mean = 0;
			std_dev = 0;
		}

		/** return what type of result we are.
		 *
		 * @return result type
		 */
		virtual EEvaluationResultType get_result_type() const
		{
			return CROSSVALIDATION_RESULT;
		}

		/** Returns the name of the SGSerializable instance.  It MUST BE
		 *  the CLASS NAME without the prefixed `C'.
		 *
		 *  @return name of the SGSerializable
		 */
		virtual const char* get_name() const
		{
			return "CrossValidationResult";
		}

		/** helper method used to specialize a base class instance
		 *
		 * @param eval_result its dynamic type must be CCrossValidationResult
		 */
		static CCrossValidationResult*
		obtain_from_generic(CEvaluationResult* eval_result)
		{
			if (!eval_result)
				return NULL;

			REQUIRE(
			    eval_result->get_result_type() == CROSSVALIDATION_RESULT,
			    "CrossValidationResult::obtain_from_generic(): argument is"
			    "of wrong type!\n");

			SG_REF(eval_result);
			return (CCrossValidationResult*)eval_result;
		}

		/** print result */
		virtual void print_result()
		{
			SG_SPRINT("%f+-%f\n", mean, std_dev);
		}

		/**
		 * Get the evaluations mean.
		 * @return mean
		 */
		float64_t get_mean() const
		{
			return mean;
		}

		/**
		 * Get the standard deviation.
		 * @return standard deviation
		 */
		float64_t get_std_dev() const
		{
			return std_dev;
		}

		/**
		 * Set the evaluations mean.
		 * @param mean the mean
		 */
		void set_mean(float64_t ev_mean)
		{
			this->mean = ev_mean;
		}

		/**
		 * Set the standard deviation
		 * @param std_dev the standard deviation
		 */
		void set_std_dev(float64_t ev_std_dev)
		{
			this->std_dev = ev_std_dev;
		}

	private:
		/** mean */
		float64_t mean;
		/** Standard deviation of cross-validation folds */
		float64_t std_dev;
	};

	/** @brief base class for cross-validation evaluation.
	 * Given a learning machine, a splitting strategy, an evaluation criterion,
	 * features and corresponding labels, this provides an interface for
	 * cross-validation. Results may be retrieved using the evaluate method. A
	 * number of repetitions may be specified for obtaining more accurate
	 * results.
	 * The arithmetic mean and standard deviation of different runs is returned.
	 * Default number of runs is one.
	 *
	 * This class calculates an evaluation criterion of every fold and then
	 * calculates the arithmetic mean of all folds. This is for example suitable
	 * for the AUC or for Accuracy. However, for example F1-measure may not be
	 * merged this way (result will be biased). To solve this, different
	 * sub-classes
	 * may average results of each cross validation fold differently by
	 * overwriting
	 * the evaluate_one_run method.
	 *
	 * See [Forman, G. and Scholz, M. (2009). Apples-to-apples in
	 * cross-validation
	 * studies: Pitfalls in classifier performance measurement. Technical
	 * report,
	 * HP Laboratories.] for details on this subject.
	 *
	 * Cross validation tries to lock underlying machines if that is possible to
	 * speed up computations. Can be turned off by the set_autolock()  method.
	 * Locking in general may speed up things (eg for kernel machines the kernel
	 * matrix is precomputed), however, it is not always supported.
	 *
	 * Crossvalidation runs with current number of threads
	 * (Parallel::set_num_threads) for unlocked case, and currently duplicates
	 * all
	 * objects (might be changed later).
	 *
	 */
	class CCrossValidation : public CMachineEvaluation
	{
	public:
		/** constructor */
		CCrossValidation();

		/** constructor
		 * @param machine learning machine to use
		 * @param features features to use for cross-validation
		 * @param labels labels that correspond to the features
		 * @param splitting_strategy splitting strategy to use
		 * @param evaluation_criterion evaluation criterion to use
		 * @param autolock whether machine should be auto-locked before
		 * evaluation
		 */
		CCrossValidation(
		    CMachine* machine, CFeatures* features, CLabels* labels,
		    CSplittingStrategy* splitting_strategy,
		    CEvaluation* evaluation_criterion, bool autolock = true);

		/** constructor, for use with custom kernels (no features)
		 * @param machine learning machine to use
		 * @param labels labels that correspond to the features
		 * @param splitting_strategy splitting strategy to use
		 * @param evaluation_criterion evaluation criterion to use
		 * @param autolock autolock
		 */
		CCrossValidation(
		    CMachine* machine, CLabels* labels,
		    CSplittingStrategy* splitting_strategy,
		    CEvaluation* evaluation_criterion, bool autolock = true);

		/** destructor */
		virtual ~CCrossValidation();

		/** setter for the number of runs to use for evaluation */
		void set_num_runs(int32_t num_runs);

		/** @return name of the SGSerializable */
		virtual const char* get_name() const
		{
			return "CrossValidation";
		}

	private:
		void init();

	protected:
		/**
		 * Does the actual evaluation.
		 * @return the cross-validation result
		 */
		virtual CEvaluationResult* evaluate_impl();

	protected:
	protected:
		/** Evaluates one single cross-validation run.
		 * Current implementation evaluates each fold separately and then
		 * calculates
		 * arithmetic mean. Suitable for accuracy and AUC for example. NOT for
		 * F1-measure. Has to be overridden by sub-classes if results have to be
		 * merged differently
		 *
		 * @return evaluation result of one cross-validation run
		 */
		virtual float64_t
		evaluate_one_run(int64_t index, CrossValidationStorage* storage);

		/** number of evaluation runs for one fold */
		int32_t m_num_runs;
	};
}

#endif /* __CROSSVALIDATION_H_ */
