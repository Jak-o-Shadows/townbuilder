Gap Analysis
============

This report identifies coverage gaps in the specification.

Requirements Without Implementations
-------------------------------------

.. needtable::
   :types: req
   :filter: len(implemented_by) == 0
   :style: table
   :columns: id;title;status;priority

Specifications Without Tests
----------------------------

.. needtable::
   :types: spec
   :filter: len(verified_by) == 0
   :style: table
   :columns: id;title;status

Failing or Draft Tests
----------------------

.. needtable::
   :types: test
   :filter: status != "pass"
   :style: table
   :columns: id;title;status;test_type

Unmitigated Risks
-----------------

.. needtable::
   :types: risk
   :filter: status == "identified"
   :style: table
   :columns: id;title;severity