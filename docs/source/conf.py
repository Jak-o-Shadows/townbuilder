# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
import sys

project = 'townbuilder'
copyright = '2026, Jak-o-Shadows'
author = 'Jak-o-Shadows'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx_needs",
    "sphinx_codelinks",
]

templates_path = ['_templates']
exclude_patterns = []

needs_from_toml = "ubproject.toml"

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_static_path = ['_static']

# Must switch between graphviz & plantUML, depending on whether we are doing latex export or not
build_type = sys.argv[2]  # This is not a great way, but is easy - see https://stackoverflow.com/a/65849575
match build_type:
    case "latex":
        needs_flow_engine = "graphviz" # Can't get a workign install of plantUML with other deps
        #needs_flow_engine = "plantuml"  # Doing latex export, so plantUML is required
        #extensions.append("sphinxcontrib.plantuml")
    case "html":
        needs_flow_engine = "graphviz"  # Not doing latex export, so graphviz is better
    case _:
        needs_flow_engine = "graphviz"  # More likely to be installed?


