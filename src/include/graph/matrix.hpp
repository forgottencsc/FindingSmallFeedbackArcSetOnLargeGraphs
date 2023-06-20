#ifndef GRAPH_MATRIX_HPP
#define GRAPH_MATRIX_HPP

namespace graph {

template<class VertexSizeType, class EdgeSizeType, class EdgeWeightType>
struct matrix_graph_traits {
    
    typedef VertexSizeType vertex_id_type;

    typedef EdgeSizeType edge_id_type;

    typedef EdgeWeightType edge_weight_type;

    typedef tagged_tuple<src<vertex_id_type>, dst<vertex_id_type>, weight<edge_weight_type>> edge_type;
    typedef tagged_tuple<src<vertex_id_type>, weight<edge_weight_type>> in_edge_type;
    typedef tagged_tuple<dst<vertex_id_type>, weight<edge_weight_type>> out_edge_type;

};


}

#endif