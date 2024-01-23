import torch
from torch_scatter import scatter
from ignite.metrics import Metric
from ignite.exceptions import NotComputableError
from ignite.metrics.metric import sync_all_reduce, reinit__is_reduced


class PerfectLCA(Metric, object):
    """
    Computes the rate of perfectly predicted LCAS matrices over a batch.

    ``output_transform`` should return the following items: ``(edge_pred, edge_y, edge_index, u_y, batch, num_graphs)``.

    * ``edge_pred`` must contain edge prediction logits and have shape (num_edges_in_batch, edge_classes);
    * ``edge_y`` must contain edge ground-truth class indices and have shape (num_edges_in_batch, 1);
    * ``edge index`` maps edges to its nodes;
    * ``u_y`` is the signal/background class (always 1 in the current setting);
    * ``batch`` maps nodes to their graph;
    * ``num_graphs`` is the number of graph in a batch (could be derived from ``batch`` also).

    .. seealso::
        `Ignite metrics <https://pytorch.org/ignite/metrics.html>`_

    :param ignore_index: Class or list of classes to ignore during the computation (e.g. padding).
    :type ignore_index: list[int]
    :param output_transform: Function to transform engine's output to desired output.
    :type output_transform: `function <https://docs.python.org/3/glossary.html#term-function>`_
    :param device: ``cpu`` or ``gpu``.
    :type device: str
    :param ignore_background: Flag to ignore background events in computation (not used).
    :type ignore_background: bool
    """

    def __init__(self, ignore_index, output_transform, device='cpu', ignore_background=False):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self.ignore_background = ignore_background
        self._per_corrects = None
        self._num_examples = None

        super(PerfectLCA, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):
        """"""
        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectLCA, self).reset()

    @reinit__is_reduced
    def update(self, output):
        """"""
        edge_pred, edge_y, edge_index, u_y, batch, num_graphs = output

        num_graphs = num_graphs.item()

        probs = torch.softmax(edge_pred, dim=1)
        winners = probs.argmax(dim=1)

        # print(y.shape)
        assert winners.shape == edge_y.shape, 'Edge predictions shape does not match target shape'

        # Create a mask for the zeroth elements (padded entries)
        mask = torch.ones(edge_y.size(), dtype=torch.long, device=self.device)
        for ig_class in self.ignore_index:
            mask &= (edge_y != ig_class)

        # Zero the respective entries in the predictions
        y_pred_mask = winners * mask
        y_mask = edge_y * mask

        # (N) compare the masked predictions with the target. The padded will be equal due to masking
        truth = y_pred_mask.eq(y_mask) + 0  # +0 so it's not bool but 0 and 1
        truth = scatter(truth, edge_index[0], reduce="min")
        truth = scatter(truth, batch, reduce="min")

        # Count the number of zero wrong predictions across the batch
        batch_perfect = truth.sum().item()

        # Ignore background events (does nothing in the current setting)
        ignored_num = torch.logical_and((u_y == 0), (truth == 1)).sum().item() if self.ignore_background else 0
        ignored_den = (u_y == 0).sum().item() if self.ignore_background else 0

        self._per_corrects += (batch_perfect - ignored_num)
        self._num_examples += (num_graphs - ignored_den)

    @sync_all_reduce("_perfectLCA")
    def compute(self):
        """"""
        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class PerfectMasses(Metric, object):
    """
    Computes the rate of events with perfectly predicted mass hypotheses over a batch.

    ``output_transform`` should return the following items: ``(x_pred, x_y, u_y, batch, num_graphs)``.

    * ``x_pred`` must contain node prediction logits and have shape (num_nodes_in_batch, node_classes);
    * ``x_y`` must contain node ground-truth class indices and have shape (num_nodes_in_batch, 1);
    * ``u_y`` is the signal/background class (always 1 in the current setting);
    * ``batch`` maps nodes to their graph;
    * ``num_graphs`` is the number of graph in a batch (could be derived from ``batch`` also).

    .. seealso::
        `Ignite metrics <https://pytorch.org/ignite/metrics.html>`_

    :param ignore_index: Class or list of classes to ignore during the computation (e.g. padding).
    :type ignore_index: list[int]
    :param output_transform: Function to transform engine's output to desired output.
    :type output_transform: `function <https://docs.python.org/3/glossary.html#term-function>`_
    :param device: ``cpu`` or ``gpu``.
    :type device: str
    :param ignore_background: Flag to ignore background events in computation (not used).
    :type ignore_background: bool
    """

    def __init__(self, ignore_index, output_transform, device='cpu', ignore_background=False):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self.ignore_background = ignore_background
        self._per_corrects = None
        self._num_examples = None

        super(PerfectMasses, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):
        """"""
        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectMasses, self).reset()

    @reinit__is_reduced
    def update(self, output):
        """"""
        x_pred, x_y, u_y, batch, num_graphs = output

        num_graphs = num_graphs.item()

        probs = torch.softmax(x_pred, dim=1)
        winners = probs.argmax(dim=1)

        assert winners.shape == x_y.shape, 'Mass predictions shape does not match target shape'

        # Create a mask for the zeroth elements (padded entries)
        mask = torch.ones(x_y.size(), dtype=torch.long, device=self.device)
        for ig_class in self.ignore_index:
            mask &= (x_y != ig_class)

        # Zero the respective entries in the predictions
        y_pred_mask = winners * mask
        y_mask = x_y * mask

        # (N) compare the masked predictions with the target. The padded will be equal due to masking
        truth = y_pred_mask.eq(y_mask) + 0  # +0 so it's not bool but 0 and 1
        truth = scatter(truth, batch, reduce="min")

        # Count the number of zero wrong predictions across the batch
        batch_perfect = truth.sum().item()

        # Ignore background events
        ignored_num = torch.logical_and((u_y == 0), (truth == 1)).sum().item() if self.ignore_background else 0
        ignored_den = (u_y == 0).sum().item() if self.ignore_background else 0

        self._per_corrects += (batch_perfect - ignored_num)
        self._num_examples += (num_graphs - ignored_den)

    @sync_all_reduce("_perfectMasses")
    def compute(self):
        """"""
        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class PerfectEvent(Metric, object):
    """
    Computes the rate of events with perfectly predicted mass hypotheses and LCAS matrices over a batch.

    ``output_transform`` should return the following items:
    ``(x_pred, x_y, edge_pred, edge_y, edge_index, u_y, batch, num_graphs)``.

    * ``x_pred`` must contain node prediction logits and have shape (num_nodes_in_batch, node_classes);
    * ``x_y`` must contain node ground-truth class indices and have shape (num_nodes_in_batch, 1);
    * ``edge_pred`` must contain edge prediction logits and have shape (num_edges_in_batch, edge_classes);
    * ``edge_y`` must contain edge ground-truth class indices and have shape (num_edges_in_batch, 1);
    * ``edge index`` maps edges to its nodes;
    * ``u_y`` is the signal/background class (always 1 in the current setting);
    * ``batch`` maps nodes to their graph;
    * ``num_graphs`` is the number of graph in a batch (could be derived from ``batch`` also).

    .. seealso::
        `Ignite metrics <https://pytorch.org/ignite/metrics.html>`_

    :param ignore_index: Class or list of classes to ignore during the computation (e.g. padding).
    :type ignore_index: list[int]
    :param output_transform: Function to transform engine's output to desired output.
    :type output_transform: `function <https://docs.python.org/3/glossary.html#term-function>`_
    :param device: ``cpu`` or ``gpu``.
    :type device: str
    :param ignore_background: Flag to ignore background events in computation (not used).
    :type ignore_background: bool
    """

    def __init__(self, ignore_index, output_transform, device='cpu', ignore_background=False):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self.ignore_background = ignore_background
        self._per_corrects = None
        self._num_examples = None

        super(PerfectEvent, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):
        """"""
        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectEvent, self).reset()

    @reinit__is_reduced
    def update(self, output):
        """"""
        x_pred, x_y, edge_pred, edge_y, edge_index, u_y, batch, num_graphs = output

        num_graphs = num_graphs.item()

        x_probs = torch.softmax(x_pred, dim=1)
        x_winners = x_probs.argmax(dim=1)
        edge_probs = torch.softmax(edge_pred, dim=1)
        edge_winners = edge_probs.argmax(dim=1)

        assert x_winners.shape == x_y.shape, 'Mass predictions shape does not match target shape'
        assert edge_winners.shape == edge_y.shape, 'Edge predictions shape does not match target shape'

        # Create a mask for the zeroth elements (padded entries)
        x_mask = torch.ones(x_y.size(), dtype=torch.long, device=self.device)
        edge_mask = torch.ones(edge_y.size(), dtype=torch.long, device=self.device)
        for ig_class in self.ignore_index:
            x_mask &= (x_y != ig_class)
            edge_mask &= (edge_y != ig_class)

        # Zero the respective entries in the predictions
        x_pred_mask = x_winners * x_mask
        x_mask = x_y * x_mask
        edge_pred_mask = edge_winners * edge_mask
        edge_mask = edge_y * edge_mask

        # (N) compare the masked predictions with the target. The padded will be equal due to masking
        # Masses
        x_truth = x_pred_mask.eq(x_mask) + 0  # +0 so it's not bool but 0 and 1
        x_truth = scatter(x_truth, batch, reduce="min")
        # Edges
        edge_truth = edge_pred_mask.eq(edge_mask) + 0  # +0 so it's not bool but 0 and 1
        edge_truth = scatter(edge_truth, edge_index[0], reduce="min")
        edge_truth = scatter(edge_truth, batch, reduce="min")

        # Count the number of zero wrong predictions across the batch
        truth = x_truth.bool() & edge_truth.bool()
        batch_perfect = (truth + 0).sum().item()

        # Ignore background events
        ignored_num = torch.logical_and((u_y == 0), (truth == 1)).sum().item() if self.ignore_background else 0
        ignored_den = (u_y == 0).sum().item() if self.ignore_background else 0

        self._per_corrects += (batch_perfect - ignored_num)
        self._num_examples += (num_graphs - ignored_den)

    @sync_all_reduce("_perfectEvent")
    def compute(self):
        """"""
        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


# class IsTrueB(Metric, object):
#     """
#     Computes the percentage of correctly identified B's
#     - `update` must receive output of the form `(u_pred, u_y, batch, num_graph)` or `{'u_pred': u_pred, 'u_y': u_y , ...}`.
#     - `u_pred` must contain logits and has shape (num_graph, 1)
#     - `u` contains ground-truth class indices and has same shape as u_pred
#     - num graph is the number of graph, it could be computed here using batch
#     """

#     def __init__(self, ignore_index, output_transform=lambda x: x, device='cpu'):

#         self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
#         self.device = device
#         self._per_corrects = None
#         self._num_examples = None

#         super(IsTrueB, self).__init__(output_transform=output_transform, device=device)

#     @reinit__is_reduced
#     def reset(self):

#         self._per_corrects = 0
#         self._num_examples = 0

#         super(IsTrueB, self).reset()

#     @reinit__is_reduced
#     def update(self, output):
#         u_pred, u_y, num_graphs = output

#         num_graphs = num_graphs.item()

#         u_pred = (torch.sigmoid(u_pred) > 0.5).double()  # If element has probability > 0.5 it's signal, else background

#         assert u_y.shape == u_pred.shape, 'Predictions shape does not match target shape'

#         # Create a mask for the padded entries
#         mask = torch.ones(u_y.size(), dtype=torch.long, device=self.device)
#         for ig_class in self.ignore_index:
#             mask &= (u_y != ig_class)

#         # Zero the respective entries in the predictions
#         u_pred_mask = u_pred * mask
#         u_mask = u_y * mask

#         # Count the number of zero wrong predictions across the batch
#         good_predictions = (u_pred_mask == u_mask).sum().item()

#         self._per_corrects += good_predictions
#         self._num_examples += num_graphs

#     @sync_all_reduce("_isTrueB")
#     def compute(self):

#         if self._num_examples == 0:
#             raise NotComputableError(
#                 "CustomAccuracy must have at least one example before it can be computed."
#             )
#         return self._per_corrects / self._num_examples
