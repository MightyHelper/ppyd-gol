#import "@preview/codelst:2.0.1": sourcecode


#set document(
  title: "Parallel Implementation of Conway's Game of Life",
  author: "Micaela Del Longo, Federico Williamson",
)
#show heading.where(level: 1): set text(size: 14pt, weight: "bold")
#show heading.where(level: 2): set text(size: 12pt, weight: "bold")
#show heading.where(level: 3): set text(size: 10pt, weight: "bold")
#show heading.where(level: 4): set text(size: 10pt, style: "italic")

#set page(
  margin: (top: 5cm, bottom: 4cm, left: 4cm, right: 4cm),
  numbering: "1/1"
)

#set text(
  font: "Times New Roman",
  size: 10pt,
  region: "GB"
)

#set par(
  justify: true,
)


#let to-verify(content) = [
  #text(fill:red, content)
  #text(fill:red, weight: "bold")[\- To be reviewed]
]

#let to-do(content) = [
  #text(fill:purple, weight: "bold")[TODO: ]
  #text(fill: purple, content)
]

#let university(name) = [
  #text(name)
]

#let title(content) = {
  text(weight: "bold", 
  size: 16pt, 
  content)
}

#let count(txt) = [
  #txt
  #if false [#str(txt).split(" ").len() words] else []
]
#set math.equation(supplement: "Formula")
#set pagebreak(weak: true)