Townbuilder
===========

Welcome to the Townbuilder project. This documentation contains the formal specifications.

This project uses `sphinx-needs <https://sphinx-needs.readthedocs.io/>`_ for traceable requirements management.

.. toctree::
   :maxdepth: 2
   :caption: Contents

   constitution/index
   features/index
   implementation/index
   reports/index


Implementation & Verification Traceability
===========================================
This matrix automatically matches our functional specifications to our actual PyTest test cases and codebase implementations.

.. needtable::
   :types: test, impl
   :columns: id, title, type, verified_by, implemented_by
   :style: line


CODELINKS
=========

C++ Source
----------

.. src-trace::
   :project: srccpp

GRAPH VIEW
==========

.. needflow::
   :types: req, spec, test, impl
   :show_legend:
   :show_link_names:

