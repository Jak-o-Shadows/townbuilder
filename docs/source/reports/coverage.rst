Coverage Report
===============

Comprehensive project traceability and coverage.

Requirement Coverage
--------------------

.. needpie:: Requirements by Status
   :labels: Open, Closed

   type == 'req' and status == 'open'
   type == 'req' and status == 'closed'

.. needpie:: Test Cases by Status
   :labels: Draft, Pass, Fail

   type == 'test' and status == 'draft'
   type == 'test' and status == 'pass'
   type == 'test' and status == 'fail'

Full Project Flow
-----------------

.. needflow:: Complete MVP traceability
   :filter: is_need
   :show_link_names:
   :config: lefttoright
   :show_legend:

Implementation Completeness
---------------------------

.. needtable::
   :types: impl
   :style: table
   :columns: id;title;status;implemented_by