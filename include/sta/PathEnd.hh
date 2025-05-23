// OpenSTA, Static Timing Analyzer
// Copyright (c) 2025, Parallax Software, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// 
// The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software.
// 
// Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 
// This notice may not be removed or altered from any source distribution.

#pragma once

#include <string>

#include "LibertyClass.hh"
#include "GraphClass.hh"
#include "SdcClass.hh"
#include "SearchClass.hh"
#include "Path.hh"
#include "StaState.hh"

namespace sta {

class StaState;
class RiseFall;
class MinMax;
class ReportPath;

// PathEnds represent search endpoints that are either unconstrained
// or constrained by a timing check, output delay, data check,
// or path delay.
//
// Class hierarchy:
// PathEnd (abstract)
//  PathEndUnconstrained
//  PathEndClkConstrained (abstract)
//   PathEndPathDelay (clock is optional)
//   PathEndClkConstrainedMcp (abstract)
//    PathEndCheck
//     PathEndLatchCheck
//    PathEndOutputDelay
//    PathEndGatedClock
//    PathEndDataCheck
//
class PathEnd
{
public:
  enum Type { unconstrained,
	      check,
	      data_check,
	      latch_check,
	      output_delay,
	      gated_clk,
	      path_delay
  };

  virtual PathEnd *copy() = 0;
  virtual ~PathEnd();
  void deletePath();
  Path *path() { return path_; }
  const Path *path() const { return path_; }
  virtual void setPath(Path *path);
  Vertex *vertex(const StaState *sta) const;
  const MinMax *minMax(const StaState *sta) const;
  // Synonym for minMax().
  const EarlyLate *pathEarlyLate(const StaState *sta) const;
  virtual const EarlyLate *clkEarlyLate(const StaState *sta) const;
  const RiseFall *transition(const StaState *sta) const;
  PathAnalysisPt *pathAnalysisPt(const StaState *sta) const;
  PathAPIndex pathIndex(const StaState *sta) const;
  virtual void reportShort(const ReportPath *report) const = 0;
  virtual void reportFull(const ReportPath *report) const = 0;

  // Predicates for PathEnd type.
  // Default methods overridden by respective types.
  virtual bool isUnconstrained() const { return false; }
  virtual bool isCheck() const { return false; }
  virtual bool isDataCheck() const { return false; }
  virtual bool isLatchCheck() const { return false; }
  virtual bool isOutputDelay() const { return false; }
  virtual bool isGatedClock() const { return false; }
  virtual bool isPathDelay() const { return false; }
  virtual Type type() const = 0;
  virtual const char *typeName() const = 0;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;
  virtual Arrival dataArrivalTime(const StaState *sta) const;
  // Arrival time with source clock offset.
  Arrival dataArrivalTimeOffset(const StaState *sta) const;
  virtual Required requiredTime(const StaState *sta) const = 0;
  // Required time with source clock offset.
  virtual Required requiredTimeOffset(const StaState *sta) const;
  virtual ArcDelay margin(const StaState *sta) const = 0;
  virtual float macroClkTreeDelay(const StaState *) const { return 0.0; }
  virtual Slack slack(const StaState *sta) const = 0;
  virtual Slack slackNoCrpr(const StaState *sta) const = 0;
  virtual Arrival borrow(const StaState *sta) const;
  const ClockEdge *sourceClkEdge(const StaState *sta) const;
  // Time offset for the path start so the path begins in the correct
  // source cycle.
  virtual float sourceClkOffset(const StaState *sta) const = 0;
  virtual Delay sourceClkLatency(const StaState *sta) const;
  virtual Delay sourceClkInsertionDelay(const StaState *sta) const;
  virtual Path *targetClkPath();
  virtual const Path *targetClkPath() const;
  virtual const Clock *targetClk(const StaState *sta) const;
  virtual const ClockEdge *targetClkEdge(const StaState *sta) const;
  const RiseFall *targetClkEndTrans(const StaState *sta) const;
  // Target clock with cycle accounting and source clock offsets.
  virtual float targetClkTime(const StaState *sta) const;
  // Time offset for the target clock.
  virtual float targetClkOffset(const StaState *sta) const;
  // Target clock with source clock offset.
  virtual Arrival targetClkArrival(const StaState *sta) const;
  // Target clock tree delay.
  virtual Delay targetClkDelay(const StaState *sta) const;
  virtual Delay targetClkInsertionDelay(const StaState *sta) const;
  // Does NOT include inter-clk uncertainty.
  virtual float targetNonInterClkUncertainty(const StaState *sta) const;
  virtual float interClkUncertainty(const StaState *sta) const;
  // Target clock uncertainty + inter-clk uncertainty.
  virtual float targetClkUncertainty(const StaState *sta) const;
  virtual float targetClkMcpAdjustment(const StaState *sta) const;
  virtual const TimingRole *checkRole(const StaState *sta) const;
  const TimingRole *checkGenericRole(const StaState *sta) const;
  virtual bool pathDelayMarginIsExternal() const;
  virtual PathDelay *pathDelay() const;
  // This returns the crpr signed with respect to the check type.
  // Positive for setup, negative for hold.
  virtual Crpr checkCrpr(const StaState *sta) const;
  virtual Crpr crpr(const StaState *sta) const;
  virtual MultiCyclePath *multiCyclePath() const;
  virtual TimingArc *checkArc() const { return nullptr; }
  // PathEndDataCheck data clock path.
  virtual const Path *dataClkPath() const { return nullptr; }
  virtual int setupDefaultCycles() const { return 1; }
  virtual Delay clkSkew(const StaState *sta);
  virtual bool ignoreClkLatency(const StaState * /* sta */) const { return false; }

  static bool less(const PathEnd *path_end1,
		   const PathEnd *path_end2,
		   const StaState *sta);
  static int cmp(const PathEnd *path_end1,
		 const PathEnd *path_end2,
		 const StaState *sta);
  static int cmpSlack(const PathEnd *path_end1,
		      const PathEnd *path_end2,
		      const StaState *sta);
  static int cmpArrival(const PathEnd *path_end1,
			const PathEnd *path_end2,
			const StaState *sta);
  static int cmpNoCrpr(const PathEnd *path_end1,
		       const PathEnd *path_end2,
		       const StaState *sta);

  // Helper common to multiple PathEnd classes and used
  // externally.
  // Target clock insertion delay + latency.
  static Delay checkTgtClkDelay(const Path *tgt_clk_path,
                                const ClockEdge *tgt_clk_edge,
                                const TimingRole *check_role,
                                const StaState *sta);
  static void checkTgtClkDelay(const Path *tgt_clk_path,
			       const ClockEdge *tgt_clk_edge,
			       const TimingRole *check_role,
			       const StaState *sta,
			       // Return values.
			       Delay &insertion,
			       Delay &latency);
  static float checkClkUncertainty(const ClockEdge *src_clk_edge,
				   const ClockEdge *tgt_clk_edge,
				   const Path *tgt_clk_path,
				   const TimingRole *check_role,
				   const StaState *sta);
  // Non inter-clock uncertainty.
  static float checkTgtClkUncertainty(const Path *tgt_clk_path,
                                      const ClockEdge *tgt_clk_edge,
                                      const TimingRole *check_role,
                                      const StaState *sta);
  static float checkSetupMcpAdjustment(const ClockEdge *src_clk_edge,
				       const ClockEdge *tgt_clk_edge,
				       const MultiCyclePath *mcp,
				       int default_cycles,
				       Sdc *sdc);

protected:
  PathEnd(Path *path);
  static void checkInterClkUncertainty(const ClockEdge *src_clk_edge,
				       const ClockEdge *tgt_clk_edge,
				       const TimingRole *check_role,
				       const StaState *sta,
				       float &uncertainty,
				       bool &exists);
  static float outputDelayMargin(OutputDelay *output_delay,
				 const Path *path,
				 const StaState *sta);
  static float pathDelaySrcClkOffset(const Path *path,
				     PathDelay *path_delay,
				     Arrival src_clk_arrival,
				     const StaState *sta);
  static bool ignoreClkLatency(const Path *path,
                               PathDelay *path_delay,
                               const StaState *sta);
  Path *path_;
};

class PathEndUnconstrained : public PathEnd
{
public:
  explicit PathEndUnconstrained(Path *path);
  virtual Type type() const;
  virtual const char *typeName() const;
  virtual PathEnd *copy();
  virtual void reportShort(const ReportPath *report) const;
  virtual void reportFull(const ReportPath *report) const;
  virtual bool isUnconstrained() const;
  virtual Required requiredTime(const StaState *sta) const;
  virtual Required requiredTimeOffset(const StaState *sta) const;
  virtual ArcDelay margin(const StaState *sta) const;
  virtual Slack slack(const StaState *sta) const;
  virtual Slack slackNoCrpr(const StaState *sta) const;
  virtual float sourceClkOffset(const StaState *sta) const;
};

class PathEndClkConstrained : public PathEnd
{
public:
  virtual float sourceClkOffset(const StaState *sta) const;
  virtual Delay sourceClkLatency(const StaState *sta) const;
  virtual Delay sourceClkInsertionDelay(const StaState *sta) const;
  virtual const Clock *targetClk(const StaState *sta) const;
  virtual const ClockEdge *targetClkEdge(const StaState *sta) const;
  virtual Path *targetClkPath();
  virtual const Path *targetClkPath() const;
  virtual float targetClkTime(const StaState *sta) const;
  virtual float targetClkOffset(const StaState *sta) const;
  virtual Arrival targetClkArrival(const StaState *sta) const;
  virtual Delay targetClkDelay(const StaState *sta) const;
  virtual Delay targetClkInsertionDelay(const StaState *sta) const;
  virtual float targetNonInterClkUncertainty(const StaState *sta) const;
  virtual float interClkUncertainty(const StaState *sta) const;
  virtual float targetClkUncertainty(const StaState *sta) const;
  virtual Crpr crpr(const StaState *sta) const;
  virtual Required requiredTime(const StaState *sta) const;
  virtual Slack slack(const StaState *sta) const;
  virtual Slack slackNoCrpr(const StaState *sta) const;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;
  virtual void setPath(Path *path);

protected:
  PathEndClkConstrained(Path *path,
			Path *clk_path);
  PathEndClkConstrained(Path *path,
			Path *clk_path,
			Crpr crpr,
			bool crpr_valid);

  float sourceClkOffset(const ClockEdge *src_clk_edge,
			const ClockEdge *tgt_clk_edge,
			const TimingRole *check_role,
			const StaState *sta) const;
  // Internal to slackNoCrpr.
  virtual Arrival targetClkArrivalNoCrpr(const StaState *sta) const;
  virtual Required requiredTimeNoCrpr(const StaState *sta) const;

  Path *clk_path_;
  mutable Crpr crpr_;
  mutable bool crpr_valid_;
};

class PathEndClkConstrainedMcp : public PathEndClkConstrained
{
public:
  virtual MultiCyclePath *multiCyclePath() const { return mcp_; }
  virtual float targetClkMcpAdjustment(const StaState *sta) const;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;

protected:
  PathEndClkConstrainedMcp(Path *path,
			   Path *clk_path,
			   MultiCyclePath *mcp);
  PathEndClkConstrainedMcp(Path *path,
			   Path *clk_path,
			   MultiCyclePath *mcp,
			   Crpr crpr,
			   bool crpr_valid);
  float checkMcpAdjustment(const Path *path,
			   const ClockEdge *tgt_clk_edge,
			   const StaState *sta) const;
  void findHoldMcps(const ClockEdge *tgt_clk_edge,
		    const MultiCyclePath *&setup_mcp,
		    const MultiCyclePath *&hold_mcp,
		    const StaState *sta) const;

  MultiCyclePath *mcp_;
};

// Path constrained by timing check.
class PathEndCheck : public PathEndClkConstrainedMcp
{
public:
  PathEndCheck(Path *path,
	       TimingArc *check_arc,
	       Edge *check_edge,
	       Path *clk_path,
	       MultiCyclePath *mcp,
	       const StaState *sta);
  virtual PathEnd *copy();
  virtual Type type() const;
  virtual const char *typeName() const;
  virtual void reportShort(const ReportPath *report) const;
  virtual void reportFull(const ReportPath *report) const;
  virtual bool isCheck() const { return true; }
  virtual ArcDelay margin(const StaState *sta) const;
  virtual float macroClkTreeDelay(const StaState *sta) const;
  virtual const TimingRole *checkRole(const StaState *sta) const;
  virtual TimingArc *checkArc() const { return check_arc_; }
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;
  virtual Delay clkSkew(const StaState *sta);

protected:
  PathEndCheck(Path *path,
	       TimingArc *check_arc,
	       Edge *check_edge,
	       Path *clk_path,
	       MultiCyclePath *mcp,
	       Crpr crpr,
	       bool crpr_valid);
  Delay sourceClkDelay(const StaState *sta) const;
  virtual Required requiredTimeNoCrpr(const StaState *sta) const;

  TimingArc *check_arc_;
  Edge *check_edge_;
};

// PathEndClkConstrained::clk_path_ is the latch enable.
class PathEndLatchCheck : public PathEndCheck
{
public:
  PathEndLatchCheck(Path *path,
		    TimingArc *check_arc,
		    Edge *check_edge,
		    Path *disable_path,
		    MultiCyclePath *mcp,
		    PathDelay *path_delay,
		    const StaState *sta);
  virtual Type type() const;
  virtual const char *typeName() const;
  virtual float sourceClkOffset(const StaState *sta) const;
  virtual bool isCheck() const { return false; }
  virtual bool isLatchCheck() const { return true; }
  virtual PathDelay *pathDelay() const { return path_delay_; }
  virtual PathEnd *copy();
  Path *latchDisable();
  const Path *latchDisable() const;
  virtual void reportShort(const ReportPath *report) const;
  virtual void reportFull(const ReportPath *report) const;
  virtual const TimingRole *checkRole(const StaState *sta) const;
  virtual Required requiredTime(const StaState *sta) const;
  virtual Arrival borrow(const StaState *sta) const;
  Arrival targetClkWidth(const StaState *sta) const;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;
  void latchRequired(const StaState *sta,
		     // Return values.
		     Required &required,
		     Delay &borrow,
		     Arrival &adjusted_data_arrival,
		     Delay &time_given_to_startpoint) const;
  void latchBorrowInfo(const StaState *sta,
		       // Return values.
		       float &nom_pulse_width,
		       Delay &open_latency,
		       Delay &latency_diff,
		       float &open_uncertainty,
		       Crpr &open_crpr,
		       Crpr &crpr_diff,
		       Delay &max_borrow,
		       bool &borrow_limit_exists) const;  
  virtual bool ignoreClkLatency(const StaState *sta) const;

protected:
  PathEndLatchCheck(Path *path,
		    TimingArc *check_arc,
		    Edge *check_edge,
		    Path *clk_path,
		    Path *disable,
		    MultiCyclePath *mcp,
		    PathDelay *path_delay,
		    Delay src_clk_arrival,
		    Crpr crpr,
		    bool crpr_valid);

private:
  Path *disable_path_;
  PathDelay *path_delay_;
  // Source clk arrival for set_max_delay -ignore_clk_latency.
  Arrival src_clk_arrival_;
};

// Path constrained by an output delay.
// If there is a reference pin, clk_path_ is the reference pin clock.
// If there is a path delay PathEndPathDelay is used instead of this.
class PathEndOutputDelay : public PathEndClkConstrainedMcp
{
public:
  PathEndOutputDelay(OutputDelay *output_delay,
		     Path *path,
		     Path *clk_path,
		     MultiCyclePath *mcp,
		     const StaState *sta);
  virtual PathEnd *copy();
  virtual Type type() const;
  virtual const char *typeName() const;
  virtual void reportShort(const ReportPath *report) const;
  virtual void reportFull(const ReportPath *report) const;
  virtual bool isOutputDelay() const { return true; }
  virtual ArcDelay margin(const StaState *sta) const;
  virtual const TimingRole *checkRole(const StaState *sta) const;
  virtual const ClockEdge *targetClkEdge(const StaState *sta) const;
  virtual Arrival targetClkArrivalNoCrpr(const StaState *sta) const;
  virtual Delay targetClkDelay(const StaState *sta) const;
  virtual Delay targetClkInsertionDelay(const StaState *sta) const;
  virtual Crpr crpr(const StaState *sta) const;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;

protected:
  PathEndOutputDelay(OutputDelay *output_delay,
		     Path *path,
		     Path *clk_path,
		     MultiCyclePath *mcp,
		     Crpr crpr,
		     bool crpr_valid);
  Arrival tgtClkDelay(const ClockEdge *tgt_clk_edge,
		      const TimingRole *check_role,
		      const StaState *sta) const;
  void tgtClkDelay(const ClockEdge *tgt_clk_edge,
		   const TimingRole *check_role,
		   const StaState *sta,
		   // Return values.
		   Arrival &insertion,
		   Arrival &latency) const;

  OutputDelay *output_delay_;
};

// Clock path constrained clock gating signal.
class PathEndGatedClock : public PathEndClkConstrainedMcp
{
public:
  PathEndGatedClock(Path *gating_ref,
		    Path *clk_path,
		    const TimingRole *check_role,
		    MultiCyclePath *mcp,
		    ArcDelay margin,
		    const StaState *sta);
  virtual PathEnd *copy();
  virtual Type type() const;
  virtual const char *typeName() const;
  virtual void reportShort(const ReportPath *report) const;
  virtual void reportFull(const ReportPath *report) const;
  virtual bool isGatedClock() const { return true; }
  virtual ArcDelay margin(const StaState *) const { return margin_; }
  virtual const TimingRole *checkRole(const StaState *sta) const;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;

protected:
  PathEndGatedClock(Path *gating_ref,
		    Path *clk_path,
		    const TimingRole *check_role,
		    MultiCyclePath *mcp,
		    ArcDelay margin,
		    Crpr crpr,
		    bool crpr_valid);

  const TimingRole *check_role_;
  ArcDelay margin_;
};

class PathEndDataCheck : public PathEndClkConstrainedMcp
{
public:
  PathEndDataCheck(DataCheck *check,
		   Path *data_path,
		   Path *data_clk_path,
		   MultiCyclePath *mcp,
		   const StaState *sta);
  virtual PathEnd *copy();
  virtual Type type() const;
  virtual const char *typeName() const;
  virtual void reportShort(const ReportPath *report) const;
  virtual void reportFull(const ReportPath *report) const;
  virtual bool isDataCheck() const { return true; }
  virtual const ClockEdge *targetClkEdge(const StaState *sta) const;
  virtual const TimingRole *checkRole(const StaState *sta) const;
  virtual ArcDelay margin(const StaState *sta) const;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;
  virtual const Path *dataClkPath() const { return data_clk_path_; }

protected:
  PathEndDataCheck(DataCheck *check,
		   Path *data_path,
		   Path *data_clk_path,
		   Path *clk_path,
		   MultiCyclePath *mcp,
		   Crpr crpr,
		   bool crpr_valid);
  Path *clkPath(Path *path,
                const StaState *sta);
  Arrival requiredTimeNoCrpr(const StaState *sta) const;
  // setup uses zero cycle default
  virtual int setupDefaultCycles() const { return 0; }

private:
  Path *data_clk_path_;
  DataCheck *check_;
};

// Path constrained by set_min/max_delay.
// "Clocked" when path delay ends at timing check pin.
// May end at output with set_output_delay.
class PathEndPathDelay : public PathEndClkConstrained
{
public:
  // Vanilla path delay.
  PathEndPathDelay(PathDelay *path_delay,
		   Path *path,
		   const StaState *sta);
  // Path delay to timing check.
  PathEndPathDelay(PathDelay *path_delay,
		   Path *path,
		   Path *clk_path,
		   TimingArc *check_arc,
		   Edge *check_edge,
		   const StaState *sta);
  // Path delay to output with set_output_delay.
  PathEndPathDelay(PathDelay *path_delay,
		   Path *path,
		   OutputDelay *output_delay,
		   const StaState *sta);
  virtual PathEnd *copy();
  virtual Type type() const;
  virtual const char *typeName() const;
  virtual void reportShort(const ReportPath *report) const;
  virtual void reportFull(const ReportPath *report) const;
  virtual bool isPathDelay() const { return true; }
  virtual const TimingRole *checkRole(const StaState *sta) const;
  virtual bool pathDelayMarginIsExternal() const;
  virtual PathDelay *pathDelay() const { return path_delay_; }
  virtual ArcDelay margin(const StaState *sta) const;
  virtual float sourceClkOffset(const StaState *sta) const;
  virtual const ClockEdge *targetClkEdge(const StaState *sta) const;
  virtual float targetClkTime(const StaState *sta) const;
  virtual Arrival targetClkArrivalNoCrpr(const StaState *sta) const;
  virtual float targetClkOffset(const StaState *sta) const;
  virtual TimingArc *checkArc() const { return check_arc_; }
  virtual Required requiredTime(const StaState *sta) const;
  virtual int exceptPathCmp(const PathEnd *path_end,
			    const StaState *sta) const;
  bool hasOutputDelay() const { return output_delay_ != nullptr; }
  virtual bool ignoreClkLatency(const StaState *sta) const;

protected:
  PathEndPathDelay(PathDelay *path_delay,
		   Path *path,
		   Path *clk_path,
		   TimingArc *check_arc,
		   Edge *check_edge,
		   OutputDelay *output_delay,
		   Arrival src_clk_arrival,
		   Crpr crpr,
		   bool crpr_valid);
  void findSrcClkArrival(const StaState *sta);

  PathDelay *path_delay_;
  TimingArc *check_arc_;
  Edge *check_edge_;
  // Output delay is nullptr when there is no output delay at the endpoint.
  OutputDelay *output_delay_;
  // Source clk arrival for set_min/max_delay -ignore_clk_latency.
  Arrival src_clk_arrival_;
};

////////////////////////////////////////////////////////////////

// Compare slack or arrival for unconstrained path ends and pin names,
// transitions along the source path.
class PathEndLess
{
public:
  explicit PathEndLess(const StaState *sta);
  bool operator()(const PathEnd *path_end1,
		  const PathEnd *path_end2) const;

protected:
  const StaState *sta_;
};

// Compare slack or arrival for unconstrained path ends.
class PathEndSlackLess
{
public:
  explicit PathEndSlackLess(const StaState *sta);
  bool operator()(const PathEnd *path_end1,
		  const PathEnd *path_end2) const;

protected:
  const StaState *sta_;
};

class PathEndNoCrprLess
{
public:
  explicit PathEndNoCrprLess(const StaState *sta);
  bool operator()(const PathEnd *path_end1,
		  const PathEnd *path_end2) const;

protected:
  const StaState *sta_;
};

} // namespace
